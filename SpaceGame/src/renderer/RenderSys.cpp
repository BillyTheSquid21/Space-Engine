#include "renderer/RenderSys.h"

std::vector<SGRender::System::RenderPass> SGRender::System::s_RenderPasses;
SegArray<SGRender::System::Identifier<SGRender::Shader>, 16> SGRender::System::s_Shaders;
SegArray<SGRender::System::IdCount<SGRender::Batcher>, 16> SGRender::System::s_Batchers;
SegArray<SGRender::System::IdCount<SGRender::Instancer>, 16> SGRender::System::s_Instancers;
std::unique_ptr<std::unordered_map<std::string, Geometry::Mesh>> SGRender::System::s_Models;
std::unique_ptr<std::unordered_map<std::string, Model::MatModel>> SGRender::System::s_MatModels;
std::unique_ptr<std::unordered_map<std::string, Tex::Texture>> SGRender::System::s_Textures;
std::unique_ptr<std::unordered_map<std::string, Tex::TextureAtlas>> SGRender::System::s_TexAtlases;
int32_t SGRender::System::s_Width = 640;
int32_t SGRender::System::s_Height = 320;
bool SGRender::System::s_Set = false;
const char* SGRender::System::s_DebugName = "debug";
SGRender::Lighting SGRender::System::s_Lighting;
SGRender::Camera SGRender::System::s_Camera;
SGRender::UniformBuffer SGRender::System::s_CameraBuffer;

bool SGRender::System::sortByPriority(SGRender::System::RenderPass& p1, SGRender::System::RenderPass& p2)
{
	return p1.priority > p2.priority;
}

void SGRender::System::createRenderPass(const char* passName, std::vector<Uniform>& uniforms, const char* shaderName, const char* rendererName, int16_t priority, int flag)
{
	if (!s_Set || strlen(passName) > MAX_NAME_LENGTH || strlen(shaderName) > MAX_NAME_LENGTH || strlen(rendererName) > MAX_NAME_LENGTH)
	{
		EngineLog("Invalid render pass!");
		return;
	}

	EngineLog("Building render pass : ", passName);

	char passId[MAX_NAME_LENGTH + 1] = "\0";
	strcpy_s(passId, MAX_NAME_LENGTH + 1, passName);

	//Check if pass already exists
	if (doesPassExist(passId))
	{
		EngineLog("Pass with that name already exists!");
		return;
	}

	//Check shader exists
	SGRender::Shader* shaderPtr = nullptr;
	char shaderId[MAX_NAME_LENGTH + 1] = "\0";
	strcpy_s(shaderId, MAX_NAME_LENGTH + 1, shaderName);
	for (int i = 0; i < s_Shaders.size(); i++)
	{
		if (!strcmp(shaderId, s_Shaders[i].id))
		{
			shaderPtr = &s_Shaders[i].object;
			break;
		}
	}
	if (!shaderPtr)
	{
		EngineLog("Shader not found!");
		return;
	}

	//Check renderer exists (assumes instancers are more common)
	SGRender::RendererBase* renderPtr = nullptr;
	char renderId[MAX_NAME_LENGTH + 1] = "\0";
	strcpy_s(renderId, MAX_NAME_LENGTH + 1, rendererName);
	for (int i = 0; i < s_Instancers.size(); i++)
	{
		if (!strcmp(renderId, s_Instancers[i].id))
		{
			renderPtr = &s_Instancers[i].object;
			break;
		}
	}
	if (!renderPtr)
	{
		for (int i = 0; i < s_Batchers.size(); i++)
		{
			if (!strcmp(renderId, s_Batchers[i].id))
			{
				renderPtr = &s_Batchers[i].object;
				break;
			}
		}
	}

	if (!renderPtr)
	{
		EngineLog("Renderer not found!");
		return;
	}

	//If checks passed, get uniform locs and add pass to end of passes
	shaderPtr->bind();
	std::vector<InternalUniform> unis;
	for (int i = 0; i < uniforms.size(); i++)
	{
		if (uniforms[i].type == UniformType::TEXTURE)
		{
			std::string* resolvedTex = (std::string*)uniforms[i].uniform;
			uniforms[i].uniform = findTexture(*resolvedTex);
		}
		unis.emplace_back(shaderPtr, uniforms[i]);
	}
	shaderPtr->unbind();

	s_RenderPasses.emplace_back();
	s_RenderPasses.back().priority = priority;
	s_RenderPasses.back().flag = flag;
	s_RenderPasses.back().shader = shaderPtr;
	s_RenderPasses.back().uniforms = unis;
	s_RenderPasses.back().renderer = renderPtr;

	EngineLog("Created render pass: ", passName);
	strcpy_s(s_RenderPasses.back().id, MAX_NAME_LENGTH + 1, passId);

	//Sort render passes by priority
	std::sort(s_RenderPasses.begin(), s_RenderPasses.end(), &sortByPriority);
}

void SGRender::System::removeRenderPass(const char* name)
{
	if (!s_Set || strlen(name) > MAX_NAME_LENGTH)
	{
		return;
	}

	char id[MAX_NAME_LENGTH + 1] = "\0";
	strcpy_s(id, MAX_NAME_LENGTH + 1, name);
	for (int i = 0; i < s_RenderPasses.size(); i++)
	{
		if (!strcmp(s_RenderPasses[i].id, id))
		{
			s_RenderPasses.erase(s_RenderPasses.begin() + i);
			return;
		}
	}
	EngineLog("No matching render pass was found!");
}

void SGRender::System::loadTexture(std::string path, std::string name, int slot, int bpp, int flag)
{
	if (!s_Set)
	{
		return;
	}

	if (s_Textures->find(name) == s_Textures->end())
	{
		Tex::Texture texture;
		s_Textures->insert(std::make_pair(name, texture));
		s_Textures->at(name).loadTexture(path, bpp, true);
		s_Textures->at(name).generateTexture(slot, flag);
		s_Textures->at(name).clearBuffer();
		EngineLog("Texture loaded: ", name);
		return;
	}
	EngineLog("Texture ", name, " was found");
}

void SGRender::System::loadTexture(std::string atlasName, std::string path, std::string name)
{
	if (s_TexAtlases->find(atlasName) == s_TexAtlases->end())
	{
		EngineLog("Atlas doesn't exist!");
		return;
	}

	s_TexAtlases->at(atlasName).undoTransforms();
	s_TexAtlases->at(atlasName).addTexture(name, path);
}

void SGRender::System::unloadTexture(std::string atlasName, std::string name)
{
	if (s_TexAtlases->find(atlasName) == s_TexAtlases->end())
	{
		EngineLog("Atlas doesn't exist!");
		return;
	}

	s_TexAtlases->at(atlasName).removeTexture(name);
}

void SGRender::System::unloadTexture(std::string name)
{
	if (!s_Set)
	{
		return;
	}

	if (s_Textures->find(name) != s_Textures->end())
	{
		s_Textures->at(name).deleteTexture();
		s_Textures->erase(name);
		return;
	}
	EngineLog("Texture to be deleted not found!");
}

void SGRender::System::linkModelToAtlas(std::string atlas, std::string texture, std::string model)
{
	if (s_TexAtlases->find(atlas) == s_TexAtlases->end())
	{
		EngineLog("Atlas doesn't exist!");
		return;
	}

	if (s_Models->find(model) == s_Models->end())
	{
		EngineLog("Model doesn't exist!");
		return;
	}

	s_TexAtlases->at(atlas).undoTransforms();
	s_TexAtlases->at(atlas).linkModel(texture, s_Models->at(model).getVertices(), s_Models->at(model).getVertSize());
}

void SGRender::System::unlinkModelFromAtlas(std::string atlas, std::string texture, std::string model)
{
	if (s_TexAtlases->find(atlas) == s_TexAtlases->end())
	{
		EngineLog("Atlas doesn't exist!");
		return;
	}

	if (s_Models->find(model) == s_Models->end())
	{
		EngineLog("Model doesn't exist!");
		return;
	}

	s_TexAtlases->at(atlas).undoTransforms();
	s_TexAtlases->at(atlas).unlinkModel(texture, s_Models->at(model).getVertices());
}

void SGRender::System::generateAndMapAtlas(std::string atlas, int slot, int bpp, int flag)
{
	if (s_TexAtlases->find(atlas) == s_TexAtlases->end())
	{
		EngineLog("Atlas doesn't exist!");
		return;
	}

	s_TexAtlases->at(atlas).undoTransforms();
	s_TexAtlases->at(atlas).generateTexture(slot, bpp, flag);
	s_TexAtlases->at(atlas).applyTransforms();
}

void SGRender::System::unloadModel(std::string name)
{
	if (!s_Set)
	{
		return;
	}

	if (s_Models->find(name) != s_Models->end())
	{
		s_Models->erase(name);
		return;
	}

	if (s_MatModels->find(name) != s_MatModels->end())
	{
		s_MatModels->erase(name);
		return;
	}
	EngineLog("Model to be deleted not found!");
}

int SGRender::System::loadShader(const char* vertPath, const char* fragPath, const char* name)
{
	if (!s_Set || strlen(name) > MAX_NAME_LENGTH)
	{
		EngineLog("Invalid Shader name!");
		return -1;
	}

	char id[MAX_NAME_LENGTH + 1] = "\0";
	strcpy_s(id, MAX_NAME_LENGTH + 1, name);
	for (int i = 0; i < s_Shaders.size(); i++)
	{
		if (!strcmp(s_Shaders[i].id, id))
		{
			EngineLog("Shader already loaded!");
			return i;
		}
	}
	SGRender::Shader shader;

	//Check spaces
	for (int i = 0; i < s_Shaders.size(); i++)
	{
		if (!strcmp(s_Shaders[i].id, ""))
		{
			memset(s_Shaders[i].id, 0, MAX_NAME_LENGTH + 1);
			strcpy_s(s_Shaders[i].id, MAX_NAME_LENGTH + 1, id);
			s_Shaders[i].object.create(vertPath, fragPath);
			return i;
		}
	}

	auto sha = s_Shaders.emplace_back();
	strcpy_s(sha->id, MAX_NAME_LENGTH + 1, id);
	s_Shaders.back().object.create(vertPath, fragPath); //Remember to create here as deleteprogram is called in destructor - TODO - make it not
	
	//Link to camera vp matrix
	sha->object.bindToUniformBlock("SG_ViewProjection", s_CameraBuffer.bindingPoint());
	
	return s_Shaders.size() - 1;
}

int SGRender::System::loadShader(const char* vertPath, const char* fragPath, const char* geoPath, const char* name)
{
	if (!s_Set || strlen(name) > MAX_NAME_LENGTH)
	{
		EngineLog("Invalid Shader name!");
		return -1;
	}

	char id[MAX_NAME_LENGTH + 1] = "\0";
	strcpy_s(id, MAX_NAME_LENGTH + 1, name);
	for (int i = 0; i < s_Shaders.size(); i++)
	{
		if (!strcmp(s_Shaders[i].id, id))
		{
			EngineLog("Shader already loaded!");
			return i;
		}
	}
	SGRender::Shader shader;

	//Check spaces
	for (int i = 0; i < s_Shaders.size(); i++)
	{
		if (!strcmp(s_Shaders[i].id, ""))
		{
			memset(s_Shaders[i].id, 0, MAX_NAME_LENGTH + 1);
			strcpy_s(s_Shaders[i].id, MAX_NAME_LENGTH + 1, id);
			s_Shaders[i].object.create(vertPath, geoPath, fragPath);
			return i;
		}
	}

	auto sha = s_Shaders.emplace_back();
	strcpy_s(sha->id, MAX_NAME_LENGTH + 1, id);
	sha->object.create(vertPath, geoPath, fragPath);

	//Link to camera vp matrix
	sha->object.bindToUniformBlock("ViewProjection", s_CameraBuffer.bindingPoint());

	return s_Shaders.size() - 1;
}

void SGRender::System::unloadShader(const char* name)
{
	if (!s_Set)
	{
		return;
	}

	char id[MAX_NAME_LENGTH + 1] = "\0";
	strcpy_s(id, MAX_NAME_LENGTH + 1, name);
	for (int i = 0; i < s_Shaders.size(); i++)
	{
		if (!strcmp(id, s_Shaders[i].id))
		{
			s_Shaders[i].object.deleteShader();
			memset(s_Shaders[i].id, 0, MAX_NAME_LENGTH + 1);
			removeRedundantPasses(&s_Shaders[i].object);
			return;
		}
	}
	EngineLog("Shader isn't loaded!");
}

bool SGRender::System::linkToBatcher(const char* name, uint32_t& index)
{
	if (!s_Set)
	{
		return false;
	}

	char id[MAX_NAME_LENGTH + 1] = "\0";
	strcpy_s(id, MAX_NAME_LENGTH + 1, name);
	for (int i = 0; i < s_Batchers.size(); i++)
	{
		if (!strcmp(id, s_Batchers[i].id))
		{
			index = i;
			s_Batchers[i].count++;
			return true;
		}
	}
	EngineLog("Batcher not found for linking!");
	return false;
}

bool SGRender::System::linkToInstancer(const char* name, uint32_t& index)
{
	if (!s_Set)
	{
		return false;
	}

	char id[MAX_NAME_LENGTH + 1] = "\0";
	strcpy_s(id, MAX_NAME_LENGTH + 1, name);
	for (int i = 0; i < s_Instancers.size(); i++)
	{
		if (!strcmp(id, s_Instancers[i].id))
		{
			index = i;
			s_Instancers[i].count++;
			return true;
		}
	}
	EngineLog("Instancer not found for linking!");
	return false;
}

void SGRender::System::removeRedundantPasses(void* addr)
{
	for (int i = 0; i < s_RenderPasses.size(); i++)
	{
		if (s_RenderPasses[i].renderer == addr || s_RenderPasses[i].shader == addr)
		{
			EngineLog("Pass removed: ", s_RenderPasses[i].id);
			s_RenderPasses.erase(s_RenderPasses.begin() + i);
			i--;
		}
	}
}

void SGRender::System::unlinkFromBatcher(uint32_t index)
{
	if (!s_Set || index >= s_Batchers.size())
	{
		return;
	}

	s_Batchers[index].count--;
	if (s_Batchers[index].count < 1)
	{
		memset(s_Batchers[index].id, 0, MAX_NAME_LENGTH + 1);
		removeRedundantPasses(&s_Batchers[index].object);
	}
}

void SGRender::System::unlinkFromInstancer(uint32_t index)
{
	if (!s_Set)
	{
		return;
	}

	s_Instancers[index].count--;
	if (s_Instancers[index].count < 1)
	{
		memset(s_Instancers[index].id, 0, MAX_NAME_LENGTH + 1);
		removeRedundantPasses(&s_Instancers[index].object);
	}
}

void SGRender::System::commitToBatcher(int index, void* src, float* vert, unsigned int vertSize, const unsigned int* ind, unsigned int indSize)
{
	if (!s_Set)
	{
		return;
	}

	assert(index < s_Batchers.size() && "Index out of range!");
	s_Batchers[index].object.commitAndBatch(src, vert, vertSize, ind, indSize);
}

void SGRender::System::commitToInstancer(int index, void* data, unsigned int dataSize, unsigned int count)
{
	if (!s_Set)
	{
		return;
	}
	assert(index < s_Instancers.size() && "Index out of range!");
	s_Instancers[index].object.commitInstance(data, dataSize, count);
}

void SGRender::System::removeFromBatcher(int index, void* src, void* vert)
{
	if (!s_Set)
	{
		return;
	}
	assert(index < s_Batchers.size() && "Index out of range!");
	s_Batchers[index].object.remove(src, vert);
}

void SGRender::System::batch(int index)
{
	if (!s_Set)
	{
		return;
	}
	assert(index < s_Batchers.size() && "Index out of range!");
	s_Batchers[index].object.batch();
}

void SGRender::System::bufferVideoData()
{
	if (!s_Set)
	{
		return;
	}

	for (int i = 0; i < s_RenderPasses.size(); i++)
	{
		s_RenderPasses[i].renderer->bufferVideoData();
	}
}

void SGRender::System::render()
{
	if (!s_Set || s_RenderPasses.size() <= 0)
	{
		return;
	}

	s_Camera.calcVP();
	s_Camera.updateFrustum();
	glm::mat4 cam = s_Camera.getVP();
	s_CameraBuffer.bufferData(&cam, sizeof(glm::mat4));

	bool lastIndexCleared = false;
	for (int i = 0; i < s_RenderPasses.size(); i++)
	{
		RenderPass pass = s_RenderPasses[i];
		pass.shader->bind();
		//Set uniforms
		for (int u = 0; u < pass.uniforms.size(); u++)
		{
			InternalUniform uni = pass.uniforms[u];
			pass.shader->setUniform(uni.location, uni.uniform, uni.type);
		}
		flagStart(pass.flag);
		pass.renderer->drawPrimitives();
		flagEnd(pass.flag);
	}

	//Check if last shader, batcher or instancer is cleared, then shrink that array
	//Should avoid excess cleared spots
	int shaderLast = s_Shaders.size() - 1;
	if (s_Shaders.size() > 0 && s_Shaders[shaderLast].id == 0)
	{
		s_Shaders.resize(shaderLast);
	}
	int batchLast = s_Batchers.size() - 1;
	if (s_Batchers.size() > 0 && s_Batchers[batchLast].id == 0)
	{
		s_Batchers.resize(batchLast);
	}
	int instanceLast = s_Instancers.size() - 1;
	if (s_Instancers.size() > 0 && s_Instancers[instanceLast].id == 0)
	{
		s_Instancers.resize(instanceLast);
	}
}

void SGRender::System::flagStart(int flag)
{
	if (flag & D_DISABLE_DEPTH_TEST)
	{
		glDisable(GL_DEPTH_TEST);
	}
}

void SGRender::System::flagEnd(int flag)
{
	if (flag & D_DISABLE_DEPTH_TEST)
	{
		glEnable(GL_DEPTH_TEST);
	}
}

Tex::Texture* SGRender::System::findTexture(std::string name)
{
	//Checks both textures and tex atlas
	if (s_Textures->find(name) != s_Textures->end())
	{
		return &s_Textures->at(name);
	}
	if (s_TexAtlases->find(name) != s_TexAtlases->end())
	{
		return s_TexAtlases->at(name).texture();
	}

	EngineLog("Texture not found!");
	return &s_Textures->at("debug");
}

bool SGRender::System::accessModel(std::string name, Geometry::Mesh** model)
{
	if (s_Models->find(name) != s_Models->end())
	{
		*model = &s_Models->at(name);
		return true;
	}
	EngineLog("Model wasn't found!");
	return false;
}

bool SGRender::System::accessMatModel(std::string name, Model::MatModel** model)
{
	if (s_MatModels->find(name) != s_MatModels->end())
	{
		*model = &s_MatModels->at(name);
		return true;
	}
	EngineLog("Material Model wasn't found!");
	return false;
}

bool SGRender::System::getShader(const char* shader, SGRender::Shader** shaderPtr)
{
	char id[MAX_NAME_LENGTH + 1];
	strcpy_s(id, MAX_NAME_LENGTH + 1, shader);
	for (int i = 0; i < s_Shaders.size(); i++)
	{
		if (!strcmp(id, s_Shaders[i].id))
		{
			*shaderPtr = &s_Shaders[i].object;
			return true;
		}
	}
	EngineLog("Shader not found! ", shader);
	return false;
}

bool SGRender::System::init(int width, int height)
{
	s_Width = width; s_Height = height;
	EngineLogOk("Render System");
	return true;
}

void SGRender::System::set()
{
	if (s_Set)
	{
		return;
	}

	//Resets to avoid spillover from previous use
	clean();

	//Assign maps
	s_Models = std::unique_ptr<std::unordered_map<std::string, Geometry::Mesh>>(new std::unordered_map<std::string, Geometry::Mesh>());
	s_MatModels = std::unique_ptr<std::unordered_map<std::string, Model::MatModel>>(new std::unordered_map<std::string, Model::MatModel>());
	s_Textures = std::unique_ptr<std::unordered_map<std::string, Tex::Texture>>(new std::unordered_map<std::string, Tex::Texture>());
	s_TexAtlases = std::unique_ptr<std::unordered_map<std::string, Tex::TextureAtlas>>(new std::unordered_map<std::string, Tex::TextureAtlas>());

	//Create a debug texture
	generateDebugTex();

	//Set lighting
	s_Lighting.set(&s_Camera);

	//Set camera
	s_CameraBuffer.create();
	s_CameraBuffer.reserveData(sizeof(glm::mat4));
	glm::mat4 cam = s_Camera.getVP();
	s_CameraBuffer.bufferData(&cam, sizeof(glm::mat4));

	s_Set = true;
}

void SGRender::System::clean()
{
	if (!s_Set)
	{
		return;
	}

	s_RenderPasses.clear();
	s_Shaders.clear();
	s_Batchers.clear();
	s_Instancers.clear();
	s_Models->clear();
	s_Models.reset();
	s_Textures->clear();
	s_Textures.reset();
	s_TexAtlases->clear();
	s_TexAtlases.reset();
	s_Lighting.clean();
	s_Set = false;
}

void SGRender::System::generateDebugTex()
{
	//Create a default texture
	std::vector<glm::i8vec3> debugRaw;
	Tex::Texture debugTex;
	constexpr int debugWidth = 8;
	debugRaw.resize(debugWidth * debugWidth);
	for (int i = 0; i < debugWidth * debugWidth; i++)
	{
		uint8_t valR = 255 * (i % 2);
		uint8_t valB = 255 - (255 * (i % 2));
		debugRaw[i] = { valR, 0, valB };
	}
	s_Textures->insert(std::make_pair(s_DebugName, debugTex));
	s_Textures->at(s_DebugName).setWidth(debugWidth);
	s_Textures->at(s_DebugName).setHeight(debugWidth);
	s_Textures->at(s_DebugName).setBPP(3);
	s_Textures->at(s_DebugName).generateTexture(0, &debugRaw[0], Tex::T_WRAP_TEXTURE);
}

bool SGRender::System::doesPassExist(const char* name)
{
	//Check if pass already exists
	for (int i = 0; i < s_RenderPasses.size(); i++)
	{
		if (!strcmp(name, s_RenderPasses[i].id))
		{
			return true;
		}
	}
	return false;
}