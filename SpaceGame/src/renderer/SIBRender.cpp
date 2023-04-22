#include "renderer/SIBRender.h"

//Default Texture slots
constexpr int32_t SLOT_DIFF = 0;
const std::string DIFF_UNI_NAME = "u_Texture";
constexpr int32_t SLOT_SPEC = 1;
constexpr int32_t SLOT_NORM = 2;
const std::string NORM_UNI_NAME = "u_NormalMap";
constexpr int32_t SLOT_SHAD = 3;

bool SGRender::SIBRender::init(float width, float height)
{
	m_Width = width; m_Height = height;

	//Set lighting
	m_Lighting.set(m_Width, m_Height, &m_Camera);

	//Set camera
	m_CameraBuffer.create();
	m_CameraBuffer.bind();
	m_CameraBuffer.reserveData((5 * sizeof(float)) + (2 * sizeof(glm::mat4)) + (sizeof(glm::vec3)));
	m_CameraBuffer.unbind();
	m_Camera.calcVP();
	m_Camera.updateFrustum();

	//Cull lighting to frustum and tiles
	m_Lighting.cullLights();

	//Gen debug
	generateDebugTex();

	return true;
}

SGRender::ShaderID SGRender::SIBRender::loadShader(std::string vertPath, std::string fragPath, std::string name)
{
	ShaderID id = m_NextShaderID;
	m_Shaders[id] = { name, std::shared_ptr<Shader>(new Shader()) };
	m_Shaders[id].shader->create(vertPath, fragPath);
	m_Shaders[id].shader->bindToUniformBlock("ViewProjection", m_CameraBuffer.bindingPoint());
	m_NextShaderID++;
	return id;
}

SGRender::ShaderID SGRender::SIBRender::loadShader(std::string vertPath, std::string geoPath, std::string fragPath, std::string name)
{
	ShaderID id = m_NextShaderID;
	m_Shaders[id] = { name, std::shared_ptr<Shader>(new Shader()) };
	m_Shaders[id].shader->create(vertPath, geoPath, fragPath);
	m_Shaders[id].shader->bindToUniformBlock("ViewProjection", m_CameraBuffer.bindingPoint()); //Assume all normal shaders use the MVP
	m_NextShaderID++;
	return id;
}

SGRender::ModelID SGRender::SIBRender::loadModel(std::string path, std::string name, VertexType vertexType, Model::MFlag flags)
{
	ModelID id = m_NextModelID;
	m_Models[id] = { name };
	Model::LoadModel(path.c_str(), *m_Models[id].model, vertexType, flags);
	m_NextModelID++;
	return id;
}

SGRender::ModelID SGRender::SIBRender::loadMatModel(std::string path, std::string name, VertexType vertexType, Model::MFlag flags)
{
	ModelID id = m_NextModelID;
	m_MatModels[id] = { name, std::shared_ptr<Model::MatModel>(new Model::MatModel()) };
	Model::LoadModel(path.c_str(), *m_MatModels[id].model, vertexType, flags);
	m_NextModelID++;	
	return id;
}

bool SGRender::SIBRender::modelExists(ModelID id)
{
	return modelExistsInternal(id) || matModelExistsInternal(id);
}

bool SGRender::SIBRender::modelExistsInternal(ModelID id)
{
	if (m_Models.count(id))
	{
		return true;
	}
	return false;
}

bool SGRender::SIBRender::matModelExistsInternal(ModelID id)
{
	if (m_MatModels.count(id))
	{
		return true;
	}
	return false;
}

SGRender::ModelID SGRender::SIBRender::locateModel(std::string name)
{
	for (auto& m : m_Models)
	{
		if (m.second.name == name)
		{
			return m.first;
		}
	}
	for (auto& m : m_MatModels)
	{
		if (m.second.name == name)
		{
			return m.first;
		}
	}
	return -1;
}

void SGRender::SIBRender::unloadModel(ModelID id)
{
	if (modelExistsInternal(id))
	{
		m_Models.erase(id);
		return;
	}

	if (matModelExistsInternal(id))
	{
		m_MatModels.erase(id);
		return;
	}
}

SGRender::TexID SGRender::SIBRender::loadTexture(std::string path, std::string name)
{
	TexID id = m_NextTexID;
	auto texture = std::shared_ptr<Tex::Texture>(new Tex::Texture());
	texture->loadTexture(path);

	if (texture->buffer() == nullptr)
	{
		EngineLog("Texture not able to be loaded! ", name, " Using debug");
		return 0; //Debug texture
	}

	m_Textures[id] = { name, texture };
	m_NextTexID++;
	return id;
}

void SGRender::SIBRender::unloadTexture(TexID id)
{
	if (!textureExists(id) || id == 0)
	{
		return;
	}
	m_Textures[id].texture->clearBuffer();
	m_Textures[id].texture->deleteTexture();
	m_Textures.erase(id);
}

bool SGRender::SIBRender::textureExists(TexID id)
{
	if (m_Textures.count(id))
	{
		return true;
	}
	return false;
}

void SGRender::SIBRender::generateTexture(TexID id, int slot, Tex::TFlag flags)
{
	if (!textureExists(id) || id == 0)
	{
		return;
	}

	m_Textures[id].texture->generateTexture(slot, flags);
}

SGRender::TexID SGRender::SIBRender::locateTexture(std::string name)
{
	for (auto& t : m_Textures)
	{
		if (t.second.name == name)
		{
			return t.first;
		}
	}
	EngineLog("Texture not able to be located: ", name, " using debug!");
	return 0; //Use debug otherwise
}

void SGRender::SIBRender::unloadShader(ShaderID id)
{
	if (!shaderExists(id))
	{
		EngineLog("Shader cannot be unloaded as was not found!");
		return;
	}
	m_Shaders.erase(id);
}

bool SGRender::SIBRender::shaderExists(ShaderID id)
{
	if (m_Shaders.count(id))
	{
		return true;
	}
	return false;
}

SGRender::ShaderID SGRender::SIBRender::locateShader(std::string name)
{
	for (auto& s : m_Shaders)
	{
		if (s.second.name == name)
		{
			return s.first;
		}
	}
	return -1;
}

bool SGRender::SIBRender::rendererExists(RendererID id)
{
	if (m_Renderers.count(id))
	{
		return true;
	}
	return false;
}

bool SGRender::SIBRender::batchedRendererExists(RendererID id)
{
	if (m_Renderers.count(id))
	{
		if (m_Renderers[id].type == 0)
		{
			return true;
		}
		return false;
	}
	return false;
}

bool SGRender::SIBRender::instancedRendererExists(RendererID id)
{
	if (m_Renderers.count(id))
	{
		if (m_Renderers[id].type == 1)
		{
			return true;
		}
		return false;
	}
	return false;
}

bool SGRender::SIBRender::passExists(RenderPassID id)
{
	for (int i = 0; i < m_RenderPasses.size(); i++)
	{
		if (m_RenderPasses[i].passID == id)
		{
			return true;
		}
	}
	return false;
}

SGRender::RenderPassID SGRender::SIBRender::createPass(std::string name, RendererID renderer, ShaderID shader)
{
	if (!batchedRendererExists(renderer) || !shaderExists(shader))
	{
		EngineLog("Renderer or Shader not found for pass!");
		return -1;
	}

	RenderPassID id = m_NextRenderPassID;
	m_RenderPasses.emplace_back(id, renderer, shader);
	m_NextRenderPassID++;
	return id;
}

void SGRender::SIBRender::removePass(RenderPassID id)
{
	for (int i = 0; i < m_RenderPasses.size(); i++)
	{
		if (m_RenderPasses[i].passID == id)
		{
			m_RenderPasses.erase(m_RenderPasses.begin() + i);
			return;
		}
	}
	EngineLog("Render Pass not found!");
	return;
}

SGRender::RendererID SGRender::SIBRender::createBatchRenderer(std::string name, VertexType vertexType, GLenum drawingmode)
{
	RendererID id = m_NextRendererID;
	std::shared_ptr<Dep_Batcher> batcher = std::shared_ptr<Dep_Batcher>(new Dep_Batcher());
	batcher->setLayout(vertexType);
	batcher->setDrawingMode(drawingmode);
	batcher->generate(m_Width, m_Height, 0);
	m_Renderers[id] = { name, 0, std::static_pointer_cast<RendererBase>(batcher) };
	m_NextRendererID++;
	return id;
}

SGRender::RendererID SGRender::SIBRender::createInstancedRenderer(std::string name, VertexType vertexType, GLenum drawingmode)
{
	RendererID id = m_NextRendererID;
	std::shared_ptr<Instancer> instancer = std::shared_ptr<Instancer>(new Instancer());
	instancer->setLayout(vertexType);
	instancer->setDrawingMode(drawingmode);
	instancer->generate(m_Width, m_Height, nullptr, 0);
	m_Renderers[id] = { name, 1, std::static_pointer_cast<RendererBase>(instancer) };
	m_NextRendererID++;
	return id;
}

void SGRender::SIBRender::removeRenderer(RendererID id)
{
	if (rendererExists(id))
	{
		m_Renderers.erase(id);
	}
}

void SGRender::SIBRender::linkPassUniform(RenderPassID id, const Uni& uniform)
{
	if (!validatePass(id))
	{
		EngineLog("Render pass was not found! Uniform wasn't linked");
		return;
	}

	//Handle texture special case
	if (uniform.type == UniformType::TEXTURE)
	{
		//Find texture from name
		TexID tex = locateTexture(*((std::string*)uniform.uniform));
		Tex::Texture* ptr = nullptr;
		if (!textureExists(tex))
		{
			return;
		}
		ptr = m_Textures[tex].texture.get();

		Uni uni = { uniform.name, uniform.type, ptr };

		ShaderID shader_id = m_RenderPasses[id].shader;
		auto& shader_ptr = m_Shaders[shader_id].shader;
		InternalUniform uni_int = InternalUniform::InternalUniform(shader_ptr.get(), uni);
		m_RenderPasses[id].uniforms.push_back(uni_int);
		return;
	}

	ShaderID shader_id = m_RenderPasses[id].shader;
	auto& shader_ptr = m_Shaders[shader_id].shader;
	InternalUniform uni_int = InternalUniform::InternalUniform(shader_ptr.get(), uniform);
	m_RenderPasses[id].uniforms.push_back(uni_int);
}

void SGRender::SIBRender::setPassUniform(RenderPassID id, const Uni& uniform)
{
	if (!validatePass(id))
	{
		return;
	}

	//Handle texture special case
	if (uniform.type == UniformType::TEXTURE)
	{
		//Find texture from name
		TexID tex = locateTexture(*((std::string*)uniform.uniform));
		Tex::Texture* ptr = nullptr;
		ptr = m_Textures[tex].texture.get();

		Uni uni = { uniform.name, uniform.type, ptr };

		ShaderID shader_id = m_RenderPasses[id].shader;
		auto& shader_ptr = m_Shaders[shader_id].shader;
		InternalUniform uni_int = InternalUniform::InternalUniform(shader_ptr.get(), uni);
		shader_ptr->setUniform(uni_int.location, uni_int.uniform, uni_int.type);
		return;
	}
}

void SGRender::SIBRender::unlinkPassUniform(RenderPassID id, std::string name)
{
	if (!validatePass(id))
	{
		return;
	}

	//Get with internal id
	ShaderID shader_id = m_RenderPasses[id].shader;
	auto& shader_ptr = m_Shaders[shader_id].shader;
	GLuint uni_loc = shader_ptr->getUniformLocation(name);
	
	auto& uni_vector = m_RenderPasses[id].uniforms;
	for (int i = 0; i < uni_vector.size(); i++)
	{
		if (uni_vector[i].location == uni_loc)
		{
			uni_vector.erase(uni_vector.begin() + i);
			return;
		}
	}

	EngineLog("Uniform not found in render pass!");
}

bool SGRender::SIBRender::validatePass(RenderPassID id)
{
	//Check exists, then shader exists, then renderer exists
	if (!passExists(id))
	{
		EngineLog("Pass doesn't exist so cannot be validated!");
		return false;
	}

	if (!shaderExists(m_RenderPasses[id].shader))
	{
		EngineLog("Pass shader no longer exists!");
		return false;
	}

	if (!batchedRendererExists(m_RenderPasses[id].renderer)
		&& !instancedRendererExists(m_RenderPasses[id].renderer))
	{
		EngineLog("Pass renderer no longer exists!");
		return false;
	}
	return true;
}

SGRender::RenderLinkID SGRender::SIBRender::linkToBatcher(RendererID renderer, ModelID model)
{
	bool modelExists = modelExistsInternal(model);

	if (batchedRendererExists(renderer))
	{
		if (m_Renderers[renderer].type != 0)
		{
			return -1;
		}

		Batcher* b = (Batcher*)m_Renderers[renderer].renderer.get();
		
		if (modelExists)
		{
			auto& m = m_Models[model];
			return b->linkToBatcher(m.model->mesh);
		}
	}
	return -1;
}

void SGRender::SIBRender::render()
{
	if (m_RenderPasses.size() <= 0)
	{
		return;
	}

	//Set camera uniforms
	m_Camera.calcVP();
	m_Camera.updateFrustum();
	m_Camera.buffer(m_CameraBuffer);

	//Do lighting
	m_Lighting.cullLights();

	for (auto& p : m_RenderPasses)
	{
		auto& s = m_Shaders[p.shader].shader;
		s->bind();
		
		//Set uniforms
		for (int u = 0; u < p.uniforms.size(); u++)
		{
			InternalUniform uni = p.uniforms[u];
			s->setUniform(uni.location, uni.uniform, uni.type);
		}

		auto& r = m_Renderers[p.renderer].renderer;
		r->bufferVideoData();
		r->drawPrimitives();
		s->unbind();
	}
	m_Camera.resetMovementFlag();
}

void SGRender::SIBRender::clean()
{
	//delete all textures and clean all buffers
	for (auto& t : m_Textures)
	{
		t.second.texture->clearBuffer();
		t.second.texture->deleteTexture();
	}

	//clear all maps
	m_Renderers.clear();
	m_Shaders.clear();
	m_Models.clear();
	m_MatModels.clear();
	m_Textures.clear();
}

void SGRender::SIBRender::queueInstruction(RenderInstruction instr)
{
	m_Instructions.push(instr);
}

void SGRender::SIBRender::processInstructions()
{
	while (!m_Instructions.empty())
	{
		RenderInstruction instr = m_Instructions.front();
		switch (instr.instr)
		{
		case InstrType::DRAW:
			if (!rendererExists(instr.renderCall.renderer))
			{
				break;
			}
			((Batcher*)m_Renderers[instr.renderCall.renderer].renderer.get())->commit(instr.renderCall.link, *instr.renderCall.transform);
			break;
		default:
			break;
		}
		m_Instructions.pop();
	}
}

void SGRender::SIBRender::generateDebugTex()
{
	m_Textures[0].texture = std::shared_ptr<Tex::Texture>(new Tex::Texture());
	m_Textures[0].name = m_DebugName;
	m_NextTexID++;

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

	m_Textures[0].texture->setWidth(debugWidth);
	m_Textures[0].texture->setHeight(debugWidth);
	m_Textures[0].texture->setBPP(3);
	m_Textures[0].texture->generateTexture(0, &debugRaw[0], Tex::T_WRAP_TEXTURE);
}