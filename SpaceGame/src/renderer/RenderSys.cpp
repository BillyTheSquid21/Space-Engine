#include "renderer/RenderSys.h"

constexpr int32_t TYPE_BATCHER = 0;
constexpr int32_t TYPE_INSTANCER = 1;

std::shared_ptr<SGRender::MaterialMeshMap> SGRender::System::extractMatModel(Model::MatModel& model)
{
	MaterialMeshMap materialsMap;

	//Iterate each material in mesh, add if doesn't exist yet
	//Add mesh to array mapped to material
	for (auto& m : model.meshes)
	{
		MatID materialID = locateMaterial(m.mat.name);
		if (materialID == -1)
		{
			//Add material
			materialID = m_NextMaterialID;
			m_Materials[materialID] = { std::make_shared<Material::Material>(m.mat) };
			m_NextMaterialID++;
		}

		materialsMap[materialID].push_back(m.mesh);
	}

	return std::make_shared<MaterialMeshMap>(materialsMap);
}

void SGRender::System::createRenderPass(std::string name, std::vector<Uniform>& uniforms, ShaderID shader, RendererID renderer, int16_t priority, int flag)
{
	EngineLog("Building render pass : ", name);

	//Check if pass already exists
	if (doesPassExist(name))
	{
		EngineLog("Pass with that name already exists!");
		return;
	}

	if (!shaderExists(shader))
	{
		EngineLog("Shader not found!");
		return;
	}

	if (!rendererExists(renderer))
	{
		EngineLog("Renderer not found!");
		return;
	}

	//If checks passed, get uniform locs and add pass to end of passes
	s_Shaders[shader].shader->bind();
	std::vector<InternalUniform> unis;
	for (int i = 0; i < uniforms.size(); i++)
	{
		if (uniforms[i].type == UniformType::TEXTURE)
		{
			std::string* resolvedTex = (std::string*)uniforms[i].uniform;
			uniforms[i].uniform = s_Textures[locateTexture(*resolvedTex)].texture.get();
		}
		unis.emplace_back(s_Shaders[shader].shader.get(), uniforms[i]);
	}
	s_Shaders[shader].shader->unbind();

	s_RenderPasses.emplace_back();
	s_RenderPasses.back().priority = priority;
	s_RenderPasses.back().flag = flag;
	s_RenderPasses.back().shader = shader;
	s_RenderPasses.back().uniforms = unis;
	s_RenderPasses.back().renderer = renderer;

	EngineLog("Created render pass: ", name);
	s_RenderPasses.back().id == name;
}

void SGRender::System::removeRenderPass(std::string name)
{
	for (int i = 0; i < s_RenderPasses.size(); i++)
	{
		if (s_RenderPasses[i].id == name)
		{
			s_RenderPasses.erase(s_RenderPasses.begin() + i);
			return;
		}
	}
	EngineLog("No matching render pass was found!");
}

SGRender::TexID SGRender::System::loadTexture(std::string path, std::string name, int slot, int bpp, int flag)
{
	TexID id = m_NextTexID;
	s_Textures[id] = { name, std::shared_ptr<Tex::Texture>(new Tex::Texture()) };
	s_Textures[id].texture->loadTexture(path, bpp, true);
	s_Textures[id].texture->generateTexture(slot, flag);
	s_Textures[id].texture->clearBuffer();
	m_NextTexID++;
	return id;
}

void SGRender::System::unloadTexture(TexID id)
{
	if (!textureExists(id))
	{
		EngineLog("Texture not found for unloading! ", id);
		return;
	}
	s_Textures[id].texture->deleteTexture();
	s_Textures.erase(id);
}

bool SGRender::System::textureExists(TexID id)
{
	return s_Textures.count(id);
}

SGRender::TexID SGRender::System::locateTexture(std::string name)
{
	for (auto& t : s_Textures)
	{
		if (t.second.name == name)
		{
			return t.first;
		}
	}
	return m_DebugID;
}

void SGRender::System::unloadModel(ModelID id)
{
	if (modelExistsInternal(id))
	{
		s_Models.erase(id);
		return;
	}
	else if (modelExistsInternal(id))
	{
		s_MatModels.erase(id);
		return;
	}
	EngineLog("Model not found for unloading! ", id);
}

SGRender::ModelID SGRender::System::locateModel(std::string name)
{
	for (auto& m : s_Models)
	{
		if (m.second.name == name)
		{
			return m.first;
		}
	}
	for (auto& m : s_MatModels)
	{
		if (m.second.name == name)
		{
			return m.first;
		}
	}
	return -1;
}

bool SGRender::System::modelExists(ModelID id)
{
	return modelExistsInternal(id) || matModelExistsInternal(id);
}

bool SGRender::System::modelExistsInternal(ModelID id)
{
	return s_Models.count(id);
}

bool SGRender::System::matModelExistsInternal(ModelID id)
{
	return s_MatModels.count(id);
}

SGRender::ShaderID SGRender::System::loadShader(std::string vertPath, std::string fragPath, std::string name)
{
	ShaderID id = m_NextShaderID;
	std::shared_ptr<Shader> shader(new Shader());
	shader->create(vertPath, fragPath);
	s_Shaders[id] = { name, shader };

	//Link to camera vp matrix and lighting
	shader->bindToUniformBlock("SG_ViewProjection", s_CameraBuffer.bindingPoint());
	shader->bindToUniformBlock("SG_Cluster", s_Lighting.clusterBindingPoint());

	return id;
}

SGRender::ShaderID SGRender::System::loadShader(std::string vertPath, std::string fragPath, std::string geoPath, std::string name)
{
	ShaderID id = m_NextShaderID;
	std::shared_ptr<Shader> shader(new Shader());
	shader->create(vertPath, geoPath, fragPath);
	s_Shaders[id] = { name, shader };

	//Link to camera vp matrix and lighting
	shader->bindToUniformBlock("SG_ViewProjection", s_CameraBuffer.bindingPoint());
	shader->bindToUniformBlock("SG_Cluster", s_Lighting.clusterBindingPoint());

	return id;
}

void SGRender::System::unloadShader(ShaderID id)
{
	if (!shaderExists(id))
	{
		EngineLog("Shader not found for unloading!");
		return;
	}
	s_Shaders.erase(id);
}

bool SGRender::System::shaderExists(ShaderID id)
{
	return s_Shaders.count(id);
}

SGRender::ShaderID SGRender::System::locateShader(std::string name)
{
	for (auto& s : s_Shaders)
	{
		if (s.second.name == name)
		{
			return s.first;
		}
	}
	return -1;
}

SGRender::ModelID SGRender::System::loadModel(std::string path, std::string name, VertexType vertexType, int modelFlags)
{
	ModelID id = m_NextModelID;
	std::shared_ptr<Model::Model> model(new Model::Model());
	if (!Model::LoadModel(path.c_str(), model->mesh, vertexType, modelFlags))
	{
		EngineLog("Model failed to load!");
		return -1;
	}

	s_Models[id] = { name, model };
	m_NextModelID++;
	return id;
}

SGRender::ModelID SGRender::System::loadMatModel(std::string path, std::string name, VertexType vertexType, int modelFlags)
{
	ModelID id = m_NextModelID;
	Model::MatModel model;
	if (!Model::LoadModel(path.c_str(), model, vertexType, modelFlags))
	{
		EngineLog("Mat Model failed to load!");
		return -1;
	}

	s_MatModels[id] = { name, extractMatModel(model) };
	m_NextModelID++;
	return id;
}

SGRender::RendererID SGRender::System::addBatcher(std::string name, VertexType vertexType, GLenum drawMode)
{
	RendererID id = m_NextRendererID;
	std::shared_ptr<Dep_Batcher> batcher(new Dep_Batcher());
	batcher->setLayout(vertexType);
	batcher->setDrawingMode(drawMode);
	batcher->generate(s_Width, s_Height, 0);
	s_Renderers[id] = { name, TYPE_BATCHER, std::static_pointer_cast<RendererBase>(batcher) };
	m_NextRendererID++;
	return id;
}

int SGRender::System::addInstancer(std::string name, std::string modelName, VertexType vertexType, GLenum drawMode)
{
	RendererID id = m_NextRendererID;
	std::shared_ptr<Instancer> instancer(new Instancer());
	instancer->setLayout(vertexType);
	instancer->setDrawingMode(drawMode);
	instancer->generate(s_Width, s_Height, &s_Models[locateModel(modelName)].model->mesh,0);
	s_Renderers[id] = { name, TYPE_INSTANCER, std::static_pointer_cast<RendererBase>(instancer) };
	m_NextRendererID++;
	return id;
}

bool SGRender::System::rendererExists(RendererID id)
{
	return s_Renderers.count(id);
}

void SGRender::System::removeRenderer(RendererID id)
{
	if (!rendererExists(id))
	{
		EngineLog("Renderer not found for removing!");
		return;
	}
	s_Renderers.erase(id); //TODO - split batcher and instancer checks
}

SGRender::RendererID SGRender::System::locateRenderer(std::string name)
{
	for (auto& r : s_Renderers)
	{
		if (r.second.name == name)
		{
			return r.first;
		}
	}
	return -1;
}

void SGRender::System::commitToBatcher(RendererID id, void* src, float* vert, unsigned int vertSize, const unsigned int* ind, unsigned int indSize)
{
	((Dep_Batcher*)s_Renderers[id].renderer.get())->commitAndBatch(src, vert, vertSize, ind, indSize);
}

void SGRender::System::commitToInstancer(RendererID id, void* data, unsigned int dataSize, unsigned int count)
{
	((Instancer*)s_Renderers[id].renderer.get())->commitInstance(data, dataSize, count);
}

void SGRender::System::removeFromBatcher(RendererID id, void* src, void* vert)
{
	((Dep_Batcher*)s_Renderers[id].renderer.get())->remove(src, vert);
}

void SGRender::System::batch(RendererID id)
{
	((Dep_Batcher*)s_Renderers[id].renderer.get())->batch();
}

void SGRender::System::bufferVideoData()
{
	for (int i = 0; i < s_RenderPasses.size(); i++)
	{
		s_Renderers[s_RenderPasses[i].renderer].renderer->bufferVideoData();
	}
}

void SGRender::System::render()
{
	if (s_RenderPasses.size() <= 0)
	{
		return;
	}

	//Buffer camera data
	s_Camera.calcVP();
	s_Camera.updateFrustum();
	s_Camera.buffer(s_CameraBuffer);

	s_Lighting.cullLights();

	bool lastIndexCleared = false;
	for (int i = 0; i < s_RenderPasses.size(); i++)
	{
		RenderPass pass = s_RenderPasses[i];
		s_Shaders[pass.shader].shader->bind();
		//Set uniforms
		for (int u = 0; u < pass.uniforms.size(); u++)
		{
			InternalUniform uni = pass.uniforms[u];
			s_Shaders[pass.shader].shader->setUniform(uni.location, uni.uniform, uni.type);
		}
		flagStart(pass.flag);
		s_Renderers[pass.renderer].renderer->drawPrimitives();
		flagEnd(pass.flag);
	}

	s_Camera.resetMovementFlag();
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

bool SGRender::System::accessModel(std::string name, Mesh** model)
{
	ModelID id = locateModel(name);
	if (id != -1)
	{
		*model = &s_Models[id].model->mesh;
		return true;
	}
	EngineLog("Model wasn't found!");
	return false;
}

bool SGRender::System::accessMatModel(std::string name, MaterialMeshMap** model)
{
	ModelID id = locateModel(name);
	if (id != -1)
	{
		*model = s_MatModels[id].meshes.get();
		return true;
	}
	EngineLog("Material Model wasn't found!");
	return false;
}

bool SGRender::System::materialExists(MatID material)
{
	return m_Materials.count(material);
}

SGRender::MatID SGRender::System::locateMaterial(std::string name)
{
	for (auto& m : m_Materials)
	{
		if (m.second.material->name == name)
		{
			return m.first;
		}
	}
	return -1;
}

bool SGRender::System::getShader(std::string shader, SGRender::Shader** shaderPtr)
{
	for (auto& s : s_Shaders)
	{
		if (s.second.name == shader)
		{
			*shaderPtr = s.second.shader.get();
			return true;
		}
	}
	EngineLog("Shader not found! ", shader);
	return false;
}

bool SGRender::System::init(int width, int height, Camera& camera)
{
	s_Width = width; s_Height = height;
	s_Camera = camera;

	//Create a debug texture
	generateDebugTex();

	//Set lighting
	s_Lighting.set(s_Width, s_Height, &s_Camera);

	//Set camera
	s_CameraBuffer.create();
	s_CameraBuffer.bind();
	s_CameraBuffer.reserveData((5 * sizeof(float)) + (2 * sizeof(glm::mat4)) + (sizeof(glm::vec3)));
	s_CameraBuffer.unbind();
	s_Camera.calcVP();
	s_Camera.updateFrustum();

	//Cull lighting to frustum and tiles
	s_Lighting.cullLights();

	EngineLogOk("Render System");
	return true;
}

void SGRender::System::clean()
{
	s_RenderPasses.clear();
	s_Shaders.clear();
	s_Renderers.clear();
	s_Models.clear();
	s_Textures.clear();
	s_Lighting.clean();
}

void SGRender::System::generateDebugTex()
{
	//Create a default texture
	std::vector<glm::i8vec3> debugRaw;
	std::shared_ptr<Tex::Texture> debugTex(new Tex::Texture());
	constexpr int debugWidth = 8;
	debugRaw.resize(debugWidth * debugWidth);
	for (int i = 0; i < debugWidth * debugWidth; i++)
	{
		uint8_t valR = 255 * (i % 2);
		uint8_t valB = 255 - (255 * (i % 2));
		debugRaw[i] = { valR, 0, valB };
	}

	debugTex->setWidth(debugWidth);
	debugTex->setHeight(debugWidth);
	debugTex->setBPP(3);
	debugTex->generateTexture(0, &debugRaw[0], Tex::T_WRAP_TEXTURE);

	TexID id = m_NextTexID;
	s_Textures[id] = { s_DebugName, debugTex };
}

bool SGRender::System::doesPassExist(std::string name)
{
	//Check if pass already exists
	for (int i = 0; i < s_RenderPasses.size(); i++)
	{
		if (name == s_RenderPasses[i].id)
		{
			return true;
		}
	}
	return false;
}