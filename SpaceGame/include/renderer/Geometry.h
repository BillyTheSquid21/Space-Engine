#pragma once
#ifndef SHAPE_FACTORY_H
#define SHAPE_FACTORY_H

#include "array"
#include "vector"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "utility/SGUtil.h"
#include "Vertex.hpp"

//Basic primitives
namespace Primitive
{	
	//Quads
	const char QUAD_VERT = 4;
	const unsigned int Q_IND[6]
	{
		0, 1, 2,
		0, 2, 3
	};
	const unsigned int Q_LINE_IND[8]
	{
		0,1,
		1,2,
		2,3,
		3,0
	};
	const char Q_IND_COUNT = 6;
	const char Q_LINE_IND_COUNT = 8;

	//Tris
	const char TRI_VERT = 3;
	const unsigned int T_IND[3]
	{
		0, 1, 2
	};	
	const char T_IND_COUNT = 3;
}

#define Quad std::array<SGRender::Vertex, 4>
#define Tex_Quad std::array<SGRender::TVertex, 4>
#define Color_Quad std::array<SGRender::CVertex, 4>
#define Norm_Tex_Quad std::array<SGRender::NTVertex, 4>

namespace Geometry
{
	//Contains the core data of a mesh (verts and inds)
	struct MeshData
	{
		MeshData() = default;
		MeshData(std::vector<float>& verts, std::vector<unsigned int>& inds) { vertices = verts; indices = inds; }

		std::vector<float> vertices;
		std::vector<unsigned int> indices;
		void clear() { vertices.clear(); indices.clear(); }
	};

	//Mesh - wrapper for mesh data
	class Mesh
	{
	public:
		template<typename VertexType>
		void load(std::vector<float>& verts, std::vector<unsigned int>& inds) { if (m_DataLoaded) { EngineLog("Data already loaded!"); return; } m_Mesh = { verts, inds }; m_DataLoaded = true; m_Properties = VertexType::properties(); }
		void unload() { if (!m_DataLoaded) { return; } m_Mesh.clear(); m_DataLoaded = false; m_Properties = false; }
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

		int getVertSize() const { return m_Mesh.vertices.size(); };
		int getIndicesCount() const { return m_Mesh.indices.size(); };
		bool isLoaded() const { return m_DataLoaded; }
		void setLoaded(bool loaded) { m_DataLoaded = loaded; }
		int properties() const { return m_Properties; }
		void setProperties(int properties) { m_Properties = properties; }
		void clear() { m_Mesh.clear(); }

	private:
		MeshData m_Mesh;
		bool m_DataLoaded;
		int m_Properties = 0;
	};

	//Struct to combine separate meshes
	struct VertexMeta
	{
		VertexMeta(float* v, int32_t f) { verts = v; count = f; }
		float* verts = nullptr;
		int32_t count = 0;
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
		MeshMeta(void* src, float* v, unsigned int vF, const unsigned int* i, unsigned short int iC) : vertexMeta(v, vF), indiceMeta(i, iC), source(src) {}
		void* source = nullptr;
		VertexMeta vertexMeta;
		IndiceMeta indiceMeta;
	};

	void BatchMeshes(std::vector<float>& destVerts, std::vector<unsigned int>& destInds, std::vector<MeshMeta>& meshes);
	void BatchMeshes(MeshData& destMesh, std::vector<MeshMeta>& meshes);

	template<typename VertexType>
	void ApplyTransform(VertexMeta mesh, glm::mat4 tranform)
	{
		int stride = VertexType::stride();
		int properties = VertexType::properties();
		for (int i = 0; i < mesh.count; i += stride)
		{
			glm::vec3* pos = (glm::vec3*)&mesh.verts[i];
			glm::vec4 pos4f = glm::vec4(*pos, 1.0f);
			pos4f = tranform * pos4f;
			*pos = glm::vec3(pos4f);
		}

		glm::mat3 noTranslation = glm::mat3(tranform);
		if (properties & SGRender::hasNormals)
		{
			int normalOffset = VertexType::normalOffset();
			for (int i = normalOffset; i < mesh.count; i += stride)
			{
				glm::vec3* norm = (glm::vec3*)&mesh.verts[i];
				*norm = noTranslation * (*norm);
			}
		}

		if (properties & SGRender::hasTangents)
		{
			int tangentOffset = VertexType::tangentOffset();
			for (int i = tangentOffset; i < mesh.count; i += stride)
			{
				glm::vec3* tan = (glm::vec3*)&mesh.verts[i];
				*tan = noTranslation * (*tan);
			}
		}
	}

	template<typename T>
	struct QuadArray
	{
		std::vector<T> quads;
		std::vector<unsigned int> indices;
		unsigned int quadCount = 0;
	};

	template<typename T>
	void GenerateQuadArrayIndices(T& quadArr)
	{
		//Buffer indices to minimise counts of data sent to render queue
		//Find total ints needed
		unsigned int quadIntCount = quadArr.quadCount * Primitive::Q_IND_COUNT;

		//Resize vector to be able to fit and init index
		quadArr.indices.resize(quadIntCount);
		unsigned int indicesIndex = 0;

		unsigned int lastLargest = -1;
		unsigned int indicesTemp[6]{ 0,1,2,0,2,3 };
		for (unsigned int i = 0; i < quadArr.quadCount; i++) {
			//Increment all by last largest - set temp to base
			std::copy(&Primitive::Q_IND[0], &Primitive::Q_IND[Primitive::Q_IND_COUNT], &indicesTemp[0]);
			for (int j = 0; j < Primitive::Q_IND_COUNT; j++) {
				indicesTemp[j] += lastLargest + 1;
			}
			//Set last largest
			lastLargest = indicesTemp[Primitive::Q_IND_COUNT - 1];
			//Copy into vector
			std::copy(&indicesTemp[0], &indicesTemp[Primitive::Q_IND_COUNT], quadArr.indices.begin() + indicesIndex);
			//Increment index
			indicesIndex += Primitive::Q_IND_COUNT;
		}
	}

	enum class Shape
	{
		//Base shapes
		TRI, QUAD, LINE
	};

	//Quad Creation
	Quad CreateQuad(float x, float y, float width, float height);
	Color_Quad CreateColorQuad(float x, float y, float width, float height, glm::vec4 color);
	Tex_Quad CreateTextureQuad(float x, float y, float width, float height, float uvX, float uvY, float uvWidth, float uvHeight);
	Norm_Tex_Quad CreateNormalTextureQuad(float x, float y, float width, float height, float uvX, float uvY, float uvWidth, float uvHeight);

	//Currently only applies to texture vertex - will update if another derived vertex needs to access uvs
	template<typename T>
	void SetQuadUV(T* verticesArray, float u, float v, float width, float height)
	{
		verticesArray[0].uvCoords.x = u; verticesArray[0].uvCoords.y = v + height;
		verticesArray[1].uvCoords.x = u + width; verticesArray[1].uvCoords.y = v + height;
		verticesArray[2].uvCoords.x = u + width; verticesArray[2].uvCoords.y = v;
		verticesArray[3].uvCoords.x = u; verticesArray[3].uvCoords.y = v;
	}
	void CalculateQuadNormals(SGRender::NTVertex* verticesArray);

	//Utility
	unsigned short int GetVerticesCount(Shape type);
	template<typename T>
	unsigned short int GetFloatCount(Shape type) {
		return (sizeof(T) / sizeof(float)) * GetVerticesCount(type);
	}

	//Rotation
	template<typename T>
	static void AxialRotateInternal(void* verticesArray, glm::vec3 rotationCentre, float angle, size_t verticeCount, SGRender::Axis axis)
	{
		using namespace SGRender;
		T* vertexPointer = (T*)verticesArray;
		glm::vec3 axisVector = { 1.0f, 0.0f, 0.0f };
		switch (axis)
		{
		case Axis::X:
			axisVector = { 1.0f, 0.0f, 0.0f };
			break;
		case Axis::Y:
			axisVector = { 0.0f, 1.0f, 0.0f };
			break;
		case Axis::Z:
			axisVector = { 0.0f, 0.0f, 1.0f };
			break;
		}

		//Translate for each vertice
		for (int i = 0; i < verticeCount; i++) {
			Vertex* vertex = (Vertex*)(void*)&vertexPointer[i];
			glm::vec3 position = { vertex->position.x - rotationCentre.x, vertex->position.y - rotationCentre.y, vertex->position.z - rotationCentre.z };
			position = glm::rotate(position, glm::radians(angle), axisVector);
			vertex->position = { position.x + rotationCentre.x, position.y + rotationCentre.y, position.z + rotationCentre.z };
		}
	}

	template<typename T>
	void AxialRotate(void* verticesArray, glm::vec3 rotationCentre, float angle, Shape type, SGRender::Axis axis)
	{
		//Set number of vertices to translate
		size_t numberOfVertices = GetVerticesCount(type);
		AxialRotateInternal<T>(verticesArray, rotationCentre, angle, numberOfVertices, axis);
	}

	template<typename T>
	void AxialRotate(void* verticesArray, glm::vec3 rotationCentre, float angle, size_t verticeCount, SGRender::Axis axis)
	{
		AxialRotateInternal<T>(verticesArray, rotationCentre, angle, verticeCount, axis);
	}
}

#endif
