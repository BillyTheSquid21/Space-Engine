#include "renderer/Mesh.h"

//batch
void SGRender::BatchMeshes(MeshData& destMesh, std::vector<MeshMeta>& meshes)
{
	BatchMeshes(destMesh.vertices, destMesh.indices, meshes);
}

void SGRender::BatchMeshes(std::vector<float>& destVerts, std::vector<uint32_t>& destInds, std::vector<MeshMeta>& meshes)
{
	if (meshes.size() <= 0)
	{
		EngineLog("No meshes to batch!");
		return;
	}

	//Get total verts and inds
	int32_t vertSize = 0;
	int32_t indSize = 0;
	for (auto& mesh : meshes)
	{
		vertSize += mesh.vertexMeta.count;
		indSize += mesh.indiceMeta.count;
	}

	//Then create buffer with space for that many floats
	std::vector<float> vertices; std::vector<uint32_t> indices;		//Buffer declaration
	vertices.resize(vertSize); indices.resize(indSize);				//Buffer resizing
	auto verticesIterator = vertices.begin(); auto indicesIterator = indices.begin();

	//Indexes
	uint32_t vertIndex = 0; uint32_t indIndex = 0;
	int32_t indiceSizeIndex = 0;
	uint32_t largestInd = 0;

	for (int i = 0; i < meshes.size(); i++)
	{
		//Get instructions from render queue
		MeshMeta instructions = meshes[i];

		//VERTICES
		const float* dataPointer = instructions.vertexMeta.verts;
		uint32_t dataSize = instructions.vertexMeta.count;

		//Copy vertices into vector		
		std::copy(&dataPointer[0], &dataPointer[dataSize], verticesIterator + vertIndex);
		vertIndex += dataSize;

		//INDICES
		const uint32_t* indDataPointer = instructions.indiceMeta.inds;
		uint32_t indDataSize = instructions.indiceMeta.count;

		//Add to vector one by one
		int currentLargest = 0;
		for (int j = 0; j < indDataSize; j++) {
			int newValue = indDataPointer[j] + largestInd;
			indices[indIndex] = newValue + indiceSizeIndex;
			indIndex++;
			if (newValue > currentLargest) {
				currentLargest = newValue;
			}
		}
		largestInd = currentLargest;

		indiceSizeIndex++;

	}
	destVerts = vertices; destInds = indices;
}

void SGRender::ApplyTransform(VertexMeta mesh, glm::mat4 tranform, VertexType type)
{
	int stride = VertexStride(type);
	for (int i = 0; i < mesh.count; i += stride)
	{
		glm::vec3* pos = (glm::vec3*)&mesh.verts[i];
		glm::vec4 pos4f = glm::vec4(*pos, 1.0f);
		pos4f = tranform * pos4f;
		*pos = glm::vec3(pos4f);
	}

	glm::mat3 noTranslation = glm::mat3(tranform);
	if (type & V_HAS_NORMALS)
	{
		int normalOffset = VertexNormalOffset(type);
		for (int i = normalOffset; i < mesh.count; i += stride)
		{
			glm::vec3* norm = (glm::vec3*)&mesh.verts[i];
			*norm = noTranslation * (*norm);
		}
	}

	if (type & V_HAS_TANGENTS)
	{
		int tangentOffset = VertexTangentOffset(type);
		for (int i = tangentOffset; i < mesh.count; i += stride)
		{
			glm::vec3* tan = (glm::vec3*)&mesh.verts[i];
			*tan = noTranslation * (*tan);
		}
	}
}