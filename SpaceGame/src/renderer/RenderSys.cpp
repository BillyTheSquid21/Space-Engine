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
			m_Materials[materialID] = { m.mat };
			m_NextMaterialID++;
		}

		materialsMap[materialID].push_back(m.mesh);
	}

	return std::make_shared<MaterialMeshMap>(materialsMap);
}

void SGRender::System::createRenderPass(std::string name, std::vector<Uniform>& uniforms, ShaderID shader, RendererID renderer, int16_t priority, int flag)
{
	EngineLog("Building render pass : ", name);

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
	s_Shaders[shader].shader.bind();
	std::vector<InternalUniform> unis;
	for (int i = 0; i < uniforms.size(); i++)
	{
		if (uniforms[i].type == UniformType::TEXTURE)
		{
			std::string* resolvedTex = (std::string*)uniforms[i].uniform;
			uniforms[i].uniform = &s_Textures[locateTexture(*resolvedTex)].texture;
		}
		unis.emplace_back(&s_Shaders[shader].shader, uniforms[i]);
	}
	s_Shaders[shader].shader.unbind();

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

void SGRender::System::generateModelRenderPass(ModelID model, ShaderID shader)
{
	if (s_MatModels.count(model))
	{
		//2.3 Setup a renderer and pass per material
		MaterialMeshMap meshMap = s_MatModels[model].meshes;
		for (auto& m : meshMap)
		{
			//2.4.a Load diffuse texture
			Material::Material& material = m_Materials[m.first].material;
			std::string diffPath = "res/s/" + material.diffuseTexture;
			std::string diffName = "diff_" + std::to_string(m.first);
			loadTexture(diffPath, diffName, 0, 3, Tex::T_FILTER_LINEAR);

			//2.4.b Load normal texture
			std::string normPath = "res/s/" + material.normalTexture;
			std::string normName = "norm_" + std::to_string(m.first);
			loadTexture(normPath, normName, 1, 3, Tex::T_FILTER_LINEAR);

			//2.5 Add batcher for material
			SGRender::RendererID renderer = addRenderer(std::to_string(m.first).c_str(), SGRender::V_UNTVertex, GL_TRIANGLES);

			//2.6 Set tex as uniform
			std::vector<SGRender::Uniform> u =
			{
				{
					"u_Texture",
					SGRender::UniformType::TEXTURE,
					&diffName
				},

				{
					"u_NormalMap",
					SGRender::UniformType::TEXTURE,
					&normName
				},

				{
					"u_Ambient",
					SGRender::UniformType::VEC3,
					&material.ambient
				},

				{
					"u_Diffuse",
					SGRender::UniformType::VEC3,
					&material.diffuse
				},

				{
					"u_Specular",
					SGRender::UniformType::VEC3,
					&material.specular
				},

				{
					"u_Shininess",
					SGRender::UniformType::FLOAT,
					&material.shininess
				}
			};
			createRenderPass(std::to_string(m.first).c_str(), u, shader, renderer, 0, 0);

			//2.7 Link to batcher
			SGRender::RenderInstruction instr = { SGRender::InstrType::DRAW };
			instr.renderCall.material = m.first;
			instr.renderCall.model = model;
			instr.renderCall.renderer = renderer;
			queueInstruction(instr);
		}
	}
}

SGRender::TexID SGRender::System::loadTexture(std::string path, std::string name, int slot, int bpp, int flag)
{
	TexID id = m_NextTexID;
	s_Textures[id] = { name, Tex::Texture() };
	s_Textures[id].texture.loadTexture(path, bpp, true);
	s_Textures[id].texture.generateTexture(slot, flag);
	s_Textures[id].texture.clearBuffer();
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
	s_Textures[id].texture.deleteTexture();
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
	Shader shader = Shader::Shader();
	shader.create(vertPath, fragPath);
	s_Shaders[id] = { name, shader };

	//Link to camera vp matrix and lighting
	shader.bindToUniformBlock("SG_ViewProjection", s_CameraBuffer.bindingPoint());
	shader.bindToUniformBlock("SG_Cluster", s_Lighting.clusterBindingPoint());

	return id;
}

SGRender::ShaderID SGRender::System::loadShader(std::string vertPath, std::string fragPath, std::string geoPath, std::string name)
{
	ShaderID id = m_NextShaderID;
	Shader shader = Shader::Shader();
	shader.create(vertPath, geoPath, fragPath);
	s_Shaders[id] = { name, shader };

	//Link to camera vp matrix and lighting
	shader.bindToUniformBlock("SG_ViewProjection", s_CameraBuffer.bindingPoint());
	shader.bindToUniformBlock("SG_Cluster", s_Lighting.clusterBindingPoint());

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
	Model::Model model = Model::Model();
	if (!Model::LoadModel(path.c_str(), model.mesh, vertexType, modelFlags))
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

	s_MatModels[id] = { name, *extractMatModel(model) };
	m_NextModelID++;
	return id;
}

SGRender::RendererID SGRender::System::addRenderer(std::string name, VertexType vertexType, GLenum drawMode)
{
	RendererID id = m_NextRendererID;
	Renderer renderer;
	renderer.setLayout(vertexType);
	renderer.setDrawingMode(drawMode);
	renderer.generate();
	s_Renderers[id] = { name, TYPE_BATCHER, renderer };
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

void SGRender::System::drawInstr(RenderCallInstr instr)
{
	if (instr.material == -1)
	{
		auto& model = s_Models[instr.model];
		auto& renderer = s_Renderers[instr.renderer];
		if (renderer.type == TYPE_BATCHER)
		{
			renderer.renderer.commit(model.model.mesh, instr.model, 0);
		}
	}
	else
	{
		auto& model = s_MatModels[instr.model].meshes.at(instr.material);
		auto& renderer = s_Renderers[instr.renderer];
		if (renderer.type == TYPE_BATCHER)
		{
			for (auto& m : model)
			{
				renderer.renderer.commit(m, instr.model, instr.material);
			}
		}
	}
}

void SGRender::System::addLightInstr(AddLightInstr instr)
{
	s_Lighting.addLight(instr.pos, instr.brightness, instr.col, instr.radius);
}

void SGRender::System::moveCamInstr(MoveCameraInstr instr)
{
	switch (instr.motionType)
	{
	case CamMotion::Move_Forward:
		s_Camera.moveForwards(instr.speed);
		break;
	case CamMotion::Move_Sideways:
		s_Camera.moveSideways(instr.speed);
		break;
	case CamMotion::Move_Up:
		s_Camera.moveUp(instr.speed);
		break;
	case CamMotion::Move_CurrentDir:
		s_Camera.moveInCurrentDirection(instr.speed);
		break;
	case CamMotion::Pan_Sideways:
		s_Camera.panSideways(instr.speed);
		break;
	default:
		break;
	}
}

void SGRender::System::processInstructions()
{
	while (!m_Instructions.empty())
	{
		RenderInstruction instr = m_Instructions.front();
		switch (instr.instr)
		{
		case InstrType::DRAW:
			drawInstr(instr.renderCall);
			break;
		case InstrType::ADD_LIGHT:
			addLightInstr(instr.lightAdd);
			break;
		case InstrType::MOVE_CAMERA:
			moveCamInstr(instr.camMove);
			break;
		default:
			break;
		}

		m_Instructions.pop();
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
		s_Shaders[pass.shader].shader.bind();
		//Set uniforms
		for (int u = 0; u < pass.uniforms.size(); u++)
		{
			InternalUniform uni = pass.uniforms[u];
			s_Shaders[pass.shader].shader.setUniform(uni.location, uni.uniform, uni.type);
		}
		s_Renderers[pass.renderer].renderer.drawPrimitives();
	}

	s_Camera.resetMovementFlag();
}

bool SGRender::System::materialExists(MatID material)
{
	return m_Materials.count(material);
}

SGRender::MatID SGRender::System::locateMaterial(std::string name)
{
	for (auto& m : m_Materials)
	{
		if (m.second.material.name == name)
		{
			return m.first;
		}
	}
	return -1;
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
	s_CameraBuffer.resize((5 * sizeof(float)) + (2 * sizeof(glm::mat4)) + (sizeof(glm::vec3)));
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
	Tex::Texture debugTex = Tex::Texture();
	constexpr int debugWidth = 8;
	debugRaw.resize(debugWidth * debugWidth);
	for (int i = 0; i < debugWidth * debugWidth; i++)
	{
		uint8_t valR = 255 * (i % 2);
		uint8_t valB = 255 - (255 * (i % 2));
		debugRaw[i] = { valR, 0, valB };
	}

	debugTex.setWidth(debugWidth);
	debugTex.setHeight(debugWidth);
	debugTex.setBPP(3);
	debugTex.generateTexture(0, &debugRaw[0], Tex::T_WRAP_TEXTURE);

	TexID id = m_NextTexID;
	s_Textures[id] = { "debug", debugTex };
}