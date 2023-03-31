#include "renderer/SIBRender.h"

SGRender::ShaderID SGRender::SIBRender::loadShader(std::string vertPath, std::string fragPath)
{
	ShaderID id = m_NextShaderID;
	m_Shaders[id] = std::shared_ptr<Shader>(new Shader());
	m_Shaders[id]->create(vertPath, fragPath);
	m_Shaders[id]->bindToUniformBlock("ViewProjection", m_CameraBuffer.bindingPoint());
	m_NextShaderID++;
	return id;
}

SGRender::ShaderID SGRender::SIBRender::loadShader(std::string vertPath, std::string geoPath, std::string fragPath)
{
	ShaderID id = m_NextShaderID;
	m_Shaders[id] = std::shared_ptr<Shader>(new Shader());
	m_Shaders[id]->create(vertPath, geoPath, fragPath);
	m_Shaders[id]->bindToUniformBlock("ViewProjection", m_CameraBuffer.bindingPoint()); //Assume all normal shaders use the MVP
	m_NextShaderID++;
	return id;
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

bool SGRender::SIBRender::batchedRendererExists(RendererID id)
{
	if (m_BatchedRenderers.count(id))
	{
		return true;
	}
	return false;
}

bool SGRender::SIBRender::instancedRendererExists(RendererID id)
{
	if (m_InstancedRenderers.count(id))
	{
		return true;
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

SGRender::RenderPassID SGRender::SIBRender::createPass(RendererID renderer, ShaderID shader)
{
	if (!instancedRendererExists(renderer) || !shaderExists(shader))
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

SGRender::RendererID SGRender::SIBRender::createBatchRenderer(VertexType vertexType, GLenum drawingmode)
{
	RendererID id = m_NextRendererID;
	Dep_Batcher batcher = Dep_Batcher::Dep_Batcher();
	batcher.setLayout(vertexType);
	batcher.setDrawingMode(drawingmode);
	batcher.generate(m_Width, m_Height, 0);
	m_BatchedRenderers[id] = std::make_shared<Dep_Batcher>(batcher);
	m_NextRendererID++;
	return id;
}

SGRender::RendererID SGRender::SIBRender::createInstancedRenderer(VertexType vertexType, GLenum drawingmode)
{
	RendererID id = m_NextRendererID;
	Instancer instancer = Instancer::Instancer();
	instancer.setLayout(vertexType);
	instancer.setDrawingMode(drawingmode);
	instancer.generate(m_Width, m_Height, nullptr, 0);
	m_InstancedRenderers[id] = std::make_shared<Instancer>(instancer);
	m_NextRendererID++;
	return id;
}

void SGRender::SIBRender::removeRenderer(RendererID id)
{
	if (batchedRendererExists(id))
	{
		m_BatchedRenderers.erase(id);
	}
	else if (instancedRendererExists(id))
	{
		m_InstancedRenderers.erase(id);
	}
}

void SGRender::SIBRender::linkPassUniform(RenderPassID id, const Uniform& uniform)
{
	if (!validatePass(id))
	{
		EngineLog("Render pass was not found! Uniform wasn't linked");
		return;
	}

	ShaderID shader_id = m_RenderPasses[id].shader;
	auto& shader_ptr = m_Shaders[shader_id];
	InternalUniform uni_int = InternalUniform::InternalUniform(shader_ptr.get(), uniform);
	m_RenderPasses[id].uniforms.push_back(uni_int);
}

void SGRender::SIBRender::setPassUniform(RenderPassID id, const Uniform& uniform)
{
	if (!validatePass(id))
	{
		return;
	}
	
	ShaderID shader_id = m_RenderPasses[id].shader;
	auto& shader_ptr = m_Shaders[shader_id];
	InternalUniform uni_int = InternalUniform::InternalUniform(shader_ptr.get(), uniform);
	shader_ptr->setUniform(uni_int.location, uni_int.uniform, uni_int.type);
}

void SGRender::SIBRender::unlinkPassUniform(RenderPassID id, std::string name)
{
	if (!validatePass(id))
	{
		return;
	}

	//Get with internal id
	ShaderID shader_id = m_RenderPasses[id].shader;
	auto& shader_ptr = m_Shaders[shader_id];
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