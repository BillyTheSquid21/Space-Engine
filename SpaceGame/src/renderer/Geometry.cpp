#include "renderer/Geometry.h"

//batch
void Geometry::BatchMeshes(MeshData& destMesh, std::vector<MeshMeta>& meshes)
{
	BatchMeshes(destMesh.vertices, destMesh.indices, meshes);
}

void Geometry::BatchMeshes(std::vector<float>& destVerts, std::vector<unsigned int>& destInds, std::vector<MeshMeta>& meshes)
{
	if (meshes.size() <= 0)
	{
		EngineLog("No meshes to batch!");
		return;
	}

	//Get total verts and inds
	int vertSize = 0;
	int indSize = 0;
	for (auto& mesh : meshes)
	{
		vertSize += mesh.vertexMeta.count;
		indSize += mesh.indiceMeta.count;
	}

	//Then create buffer with space for that many floats
	std::vector<float> vertices; std::vector<unsigned int> indices;		//Buffer declaration
	vertices.resize(vertSize); indices.resize(indSize);				//Buffer resizing
	auto verticesIterator = vertices.begin(); auto indicesIterator = indices.begin();

	//Indexes
	unsigned int vertIndex = 0; unsigned int indIndex = 0;
	int indiceSizeIndex = 0;
	unsigned int largestInd = 0;

	for (int i = 0; i < meshes.size(); i++)
	{
		//Get instructions from render queue
		MeshMeta instructions = meshes[i];

		//VERTICES
		const float* dataPointer = instructions.vertexMeta.verts;
		unsigned int dataSize = instructions.vertexMeta.count;

		//Copy vertices into vector		
		std::copy(&dataPointer[0], &dataPointer[dataSize], verticesIterator + vertIndex);
		vertIndex += dataSize;

		//INDICES
		const unsigned int* indDataPointer = instructions.indiceMeta.inds;
		unsigned int indDataSize = instructions.indiceMeta.count;

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

//Creation
Quad Geometry::CreateQuad(float x, float y, float width, float height) 
{
	using namespace SGRender;

	Vertex v0{};
	v0.position = { x, y , 0.0f };

	Vertex v1{};
	v1.position = { x + width, y,  0.0f };

	Vertex v2{};
	v2.position = { x + width, y - height,  0.0f };

	Vertex v3{};
	v3.position = { x, y - height,  0.0f };

	return { v0, v1, v2, v3 };
}

//Creation
Color_Quad Geometry::CreateColorQuad(float x, float y, float width, float height, glm::vec4 color) 
{
	using namespace SGRender;

	CVertex v0{};
	v0.position = { x, y , 0.0f };
	v0.color = color;

	CVertex v1{};
	v1.position = { x + width, y,  0.0f };
	v1.color = color;

	CVertex v2{};
	v2.position = { x + width, y - height,  0.0f };
	v2.color = color;

	CVertex v3{};
	v3.position = { x, y - height,  0.0f };
	v3.color = color;

	return { v0, v1, v2, v3 };
}

//Creation
Tex_Quad Geometry::CreateTextureQuad(float x, float y, float width, float height, float uvX, float uvY, float uvWidth, float uvHeight) 
{
	using namespace SGRender;
	TVertex v0{};
	v0.position = { x, y , 0.0f };
	v0.uvCoords = { uvX, uvY + uvHeight };

	TVertex v1{};
	v1.position = { x + width, y,  0.0f };
	v1.uvCoords = { uvX + uvWidth, uvY + uvHeight };

	TVertex v2{};
	v2.position = { x + width, y - height,  0.0f };
	v2.uvCoords = { uvX + uvWidth, uvY };

	TVertex v3{};
	v3.position = { x, y - height,  0.0f };
	v3.uvCoords = { uvX, uvY };

	return { v0, v1, v2, v3 };
}

//Creation
Norm_Tex_Quad Geometry::CreateNormalTextureQuad(float x, float y, float width, float height, float uvX, float uvY, float uvWidth, float uvHeight) 
{
	using namespace SGRender;
	//Top left
	NTVertex v0{};
	v0.position = { x, y , 0.0f };
	v0.uvCoords = { uvX, uvY + uvHeight };
	v0.normals = {0.0f, 1.0f, 0.0f};

	//Top right
	NTVertex v1{};
	v1.position = { x + width, y,  0.0f };
	v1.uvCoords = { uvX + uvWidth, uvY + uvHeight };
	v1.normals = { 0.0f, 1.0f, 0.0f };

	//Bottom right
	NTVertex v2{};
	v2.position = { x + width, y - height,  0.0f };
	v2.uvCoords = { uvX + uvWidth, uvY };
	v2.normals = { 0.0f, 1.0f, 0.0f };

	//Bottom left
	NTVertex v3{};
	v3.position = { x, y - height,  0.0f };
	v3.uvCoords = { uvX, uvY };
	v3.normals = { 0.0f, 1.0f, 0.0f };

	return { v0, v1, v2, v3 };
}

unsigned short int Geometry::GetVerticesCount(Shape type) {
	switch (type)
	{
	//Base primitives
	case Shape::TRI:
		return Primitive::TRI_VERT;
	case Shape::QUAD:
		return Primitive::QUAD_VERT;
	default:
		return 0;
	}
}

//Generate normals
void Geometry::CalculateQuadNormals(SGRender::NTVertex* verticesArray)
{
	glm::vec3 vec0 = verticesArray[2].position - verticesArray[0].position;
	glm::vec3 vec1 = verticesArray[1].position - verticesArray[0].position;
	glm::vec3 normal = glm::normalize(glm::cross(vec0,vec1));
	for (int i = 0; i < 4; i++)
	{
		verticesArray[i].normals = normal;
	}
}