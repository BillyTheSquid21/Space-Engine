#include "renderer/Renderer.h"

void SGRender::Renderer::generate()
{
	m_VA.create();	
	m_VB.create(0);	
	m_IB.create(1);
	
	//Attach to arrays buffer
	m_VA.addBuffer(m_VB, m_VBL);
	
	m_VA.bind(); 
	m_VB.unbind(); 
	m_IB.unbind();
}

void SGRender::Renderer::setDrawingMode(GLenum type)
{
	m_PrimitiveType = type;
}

void SGRender::Renderer::setLayout(VertexType type)
{
	//All Vertexes have position so add first
	//If this isn't true, there are probably more problems
	m_VBL.push<float>(3, false);

	//Check each bit for property
	for (int i = 0; i < 8; i++)
	{
		int curr_flag = type & (1UL << i);
		int flag_stride = PropertyStride((VProperties)curr_flag);
		if (flag_stride != -1)
		{
			m_VBL.push<float>(flag_stride, false);
		}
	}

	m_VertexType = type;
}

void SGRender::Renderer::commit(Mesh& mesh, ModelID modelID, MeshID meshID)
{
	//Check if is already buffered
	auto key = std::make_pair(modelID, meshID);
	if (modelExists(modelID, meshID))
	{
		return;
	}

	m_Geometry[key] = Mesh::Mesh();
	m_Geometry[key].copyInto(mesh.getMesh());
	m_Unbatched = true;
}

void SGRender::Renderer::commit(Mesh& mesh, ModelID modelID, MeshID meshID, glm::mat4& transform)
{
	//Check if is already buffered
	auto key = std::make_pair(modelID, meshID);
	if (findTransform(modelID, meshID, transform) != -1)
	{
		return;
	}

	if (!modelExists(modelID, meshID))
	{
		m_Geometry[key] = Mesh::Mesh();
		Mesh& meshref = m_Geometry[key];
		VertexMeta vertexmeta = meshref.getMeta().vertexMeta;
		m_Geometry[key].copyInto(meshref.getMesh());
	}

	m_BatchedTransforms[key].push_back(transform);
	m_Unbatched = true;
}

void SGRender::Renderer::remove(ModelID modelID, MeshID meshID)
{
	//Check if is already buffered
	auto key = std::make_pair(modelID, meshID);
	if (!modelExists(modelID, meshID))
	{
		EngineLog("Tried to remove mesh from renderer that doesn't exist!");
		return;
	}

	m_Geometry.erase(key);
	m_Unbatched = true;
}

void SGRender::Renderer::remove(ModelID modelID, MeshID meshID, glm::mat4& transform)
{
	//Check if is already buffered
	auto key = std::make_pair(modelID, meshID);
	int32_t index = findTransform(modelID, meshID, transform);
	if (index == -1)
	{
		EngineLog("Tried to remove transform from renderer that doesn't exist!");
		return;
	}

	auto& t = m_BatchedTransforms[key];
	t.erase(t.begin() + index);
	m_Unbatched = true;

	//If no transforms left, remove mesh
	if (t.size() > 0)
	{
		return;
	}

	m_Geometry.erase(key);
}

bool SGRender::Renderer::modelExists(ModelID modelID, MeshID meshID)
{
	auto key = std::make_pair(modelID, meshID);
	return m_Geometry.count(key);
}

int32_t SGRender::Renderer::findTransform(ModelID modelID, MeshID meshID, glm::mat4& transform)
{
	auto key = std::make_pair(modelID, meshID);
	if (!m_BatchedTransforms.count(key))
	{
		return -1;
	}

	for (int32_t i = 0; i < m_BatchedTransforms[key].size(); i++)
	{
		if (m_BatchedTransforms[key][i] == transform)
		{
			return i;
		}
	}
	return -1;
}

void SGRender::Renderer::batchGeometry()
{
	//Ensure binded to avoid buffering to unbound buffers
	m_VB.bind();
	m_IB.bind();

	//Reserve space for buffers
	size_t totalIndSize = 0;
	size_t totalVertSize = 0;
	for (auto& g : m_Geometry)
	{
		totalVertSize += g.second.getVertSize();
		totalIndSize += g.second.getIndicesSize();
	}

	if (totalVertSize == 0)
	{
		m_VB.unbind();
		m_IB.unbind();
		return;
	}

	m_VB.resize(totalVertSize);
	m_IB.resize(totalIndSize);

	//Iterate all meshes
	//If doesn't have any transforms, buffer segment and continue
	//If does have transforms, apply each transform, buffer segment, etc
	size_t vertexoffset = 0;
	size_t indiceoffset = 0;
	for (auto& g : m_Geometry)
	{
		if (g.second.getVertSize() <= 0 || g.second.getIndicesCount() <= 0)
		{
			EngineLogFail("Empty mesh handed to renderer!");
			return;
		}

		size_t vertsize = g.second.getVertSize();
		size_t indcount = g.second.getIndicesSize();
		if (!m_BatchedTransforms.count(g.first))
		{
			m_VB.bufferData(g.second.getVertices(), vertexoffset, vertsize);
			m_IB.bufferData(g.second.getIndices(), indiceoffset, indcount);
			vertexoffset += vertsize;
			indiceoffset += indcount;
			continue;
		}

		//Create storage for mesh, then transform, buffer, transform etc
		Mesh tmpmesh = Mesh::Mesh();
		for (auto& mat : m_BatchedTransforms[g.first])
		{
			tmpmesh.copyInto(g.second.getMesh());
			ApplyTransform(tmpmesh.getMeta().vertexMeta, mat);

			m_VB.bufferData(tmpmesh.getVertices(), vertexoffset, vertsize);
			m_IB.bufferData(tmpmesh.getIndices(), indiceoffset, indcount);
			vertexoffset += vertsize;
			indiceoffset += indcount;
		}
	}
	m_VertBuffWidth = vertexoffset;
	m_IndBuffWidth = indiceoffset;

	m_Unbatched = false;
	m_VB.unbind();
	m_IB.unbind();
}

void SGRender::Renderer::drawPrimitives()
{
	if (m_Unbatched)
	{
		batchGeometry();
	}

	if (m_VertBuffWidth == 0)
	{
		return;
	}

	m_VA.bind(); m_IB.bind();
	glDrawElements(m_PrimitiveType, m_IndBuffWidth, GL_UNSIGNED_INT, nullptr);
	m_VA.unbind(); m_IB.unbind();
}