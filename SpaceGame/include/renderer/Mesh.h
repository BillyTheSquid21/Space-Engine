#pragma once
#ifndef SHAPE_FACTORY_H
#define SHAPE_FACTORY_H

#include "array"
#include "vector"
#include "map"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "utility/SGUtil.h"
#include "Vertex.h"

namespace SGRender
{
	//Contains the core data of a mesh (verts and inds)
	struct MeshData
	{
		MeshData() = default;
		MeshData(std::vector<float>& verts, std::vector<uint32_t>& inds) { vertices = verts; indices = inds; }

		std::vector<float> vertices;
		std::vector<uint32_t> indices;
		void clear() { vertices.clear(); indices.clear(); }
	};

	//Struct to combine separate meshes
	struct VertexMeta
	{
		VertexMeta(float* v, int32_t f, VertexType t) { verts = v; count = f; type = t; }
		float* verts = nullptr;
		int32_t count = 0;
		VertexType type;
	};

	struct IndiceMeta
	{
		IndiceMeta(const uint32_t* i, int32_t c) { inds = i; count = c; }
		const uint32_t* inds = nullptr;
		int32_t count = 0;
	};

	struct MeshMeta
	{
		MeshMeta() = default;
		MeshMeta(void* src, float* v, int32_t vF, VertexType type, const uint32_t* i, int32_t iC) : vertexMeta(v, vF, type), indiceMeta(i, iC), source(src) {}
		void* source = nullptr;
		VertexMeta vertexMeta;
		IndiceMeta indiceMeta;
	};

	//Mesh - wrapper for mesh data
	class Mesh
	{
	public:
		void load(std::vector<float>& verts, std::vector<uint32_t>& inds, VertexType type) { if (m_DataLoaded) { EngineLog("Data already loaded!"); return; } m_Mesh = { verts, inds }; m_DataLoaded = true; m_VertexType = type; }
		void unload() { if (!m_DataLoaded) { return; } m_Mesh.clear(); m_DataLoaded = false; m_VertexType = VertexType::V_Vertex; }
		void copyInto(MeshData& mesh)
		{
			m_Mesh.vertices.resize(mesh.vertices.size());
			std::copy(mesh.vertices.begin(), mesh.vertices.end(), m_Mesh.vertices.begin());
			m_Mesh.indices.resize(mesh.indices.size());
			std::copy(mesh.indices.begin(), mesh.indices.end(), m_Mesh.indices.begin());
		};

		float* getVertices() { return &m_Mesh.vertices[0]; };
		unsigned int* getIndices() { return &m_Mesh.indices[0]; };

		MeshData& getMesh() { return m_Mesh; }
		MeshMeta getMeta() { return { this, getVertices(), getVertSize(), m_VertexType, getIndices(), getIndicesCount() }; }

		int32_t getVertSize() const { return m_Mesh.vertices.size(); };
		int32_t getIndicesCount() const { return m_Mesh.indices.size(); };
		bool isLoaded() const { return m_DataLoaded; }
		void setLoaded(bool loaded) { m_DataLoaded = loaded; }
		void clear() { m_Mesh.clear(); }

	private:
		MeshData m_Mesh;
		bool m_DataLoaded;
		SGRender::VertexType m_VertexType;
	};

	void BatchMeshes(std::vector<float>& destVerts, std::vector<uint32_t>& destInds, std::vector<MeshMeta>& meshes);
	void BatchMeshes(MeshData& destMesh, std::vector<MeshMeta>& meshes);
	void ApplyTransform(VertexMeta mesh, glm::mat4 tranform, VertexType type);
}

#endif
