#include "renderer/ShapeFactory.h"

//Creation
Quad CreateQuad(float x, float y, float width, float height) {

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
Color_Quad CreateColorQuad(float x, float y, float width, float height, glm::vec4 color) {

	ColorVertex v0{};
	v0.position = { x, y , 0.0f };
	v0.color = color;

	ColorVertex v1{};
	v1.position = { x + width, y,  0.0f };
	v1.color = color;

	ColorVertex v2{};
	v2.position = { x + width, y - height,  0.0f };
	v2.color = color;

	ColorVertex v3{};
	v3.position = { x, y - height,  0.0f };
	v3.color = color;

	return { v0, v1, v2, v3 };
}

//Creation
Tex_Quad CreateTextureQuad(float x, float y, float width, float height, float uvX, float uvY, float uvWidth, float uvHeight) {

	TextureVertex v0{};
	v0.position = { x, y , 0.0f };
	v0.uvCoords = { uvX, uvY + uvHeight };

	TextureVertex v1{};
	v1.position = { x + width, y,  0.0f };
	v1.uvCoords = { uvX + uvWidth, uvY + uvHeight };

	TextureVertex v2{};
	v2.position = { x + width, y - height,  0.0f };
	v2.uvCoords = { uvX + uvWidth, uvY };

	TextureVertex v3{};
	v3.position = { x, y - height,  0.0f };
	v3.uvCoords = { uvX, uvY };

	return { v0, v1, v2, v3 };
}

//Creation
Norm_Tex_Quad CreateNormalTextureQuad(float x, float y, float width, float height, float uvX, float uvY, float uvWidth, float uvHeight) {

	//Top left
	NormalTextureVertex v0{};
	v0.position = { x, y , 0.0f };
	v0.uvCoords = { uvX, uvY + uvHeight };
	v0.normals = {0.0f, 1.0f, 0.0f};

	//Top right
	NormalTextureVertex v1{};
	v1.position = { x + width, y,  0.0f };
	v1.uvCoords = { uvX + uvWidth, uvY + uvHeight };
	v1.normals = { 0.0f, 1.0f, 0.0f };

	//Bottom right
	NormalTextureVertex v2{};
	v2.position = { x + width, y - height,  0.0f };
	v2.uvCoords = { uvX + uvWidth, uvY };
	v2.normals = { 0.0f, 1.0f, 0.0f };

	//Bottom left
	NormalTextureVertex v3{};
	v3.position = { x, y - height,  0.0f };
	v3.uvCoords = { uvX, uvY };
	v3.normals = { 0.0f, 1.0f, 0.0f };

	return { v0, v1, v2, v3 };
}

unsigned short int GetVerticesCount(Shape type) {
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
void CalculateQuadNormals(NormalTextureVertex* verticesArray)
{
	glm::vec3 vec0 = verticesArray[2].position - verticesArray[0].position;
	glm::vec3 vec1 = verticesArray[1].position - verticesArray[0].position;
	glm::vec3 normal = glm::normalize(glm::cross(vec0,vec1));
	for (int i = 0; i < 4; i++)
	{
		verticesArray[i].normals = normal;
	}
}