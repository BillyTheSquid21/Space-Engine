#include "renderer/ShapeFactory.h"

//Creation
Quad CreateQuad(float x, float y, float width, float height, float uvX, float uvY, float uvWidth, float uvHeight) {

	ColorTextureVertex v0{};
	v0.position = { x, y , 0.0f };
	v0.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	v0.uvCoords = { uvX, uvY + uvHeight };

	ColorTextureVertex v1{};
	v1.position = { x + width, y,  0.0f };
	v1.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	v1.uvCoords = { uvX + uvWidth, uvY + uvHeight };

	ColorTextureVertex v2{};
	v2.position = { x + width, y - height,  0.0f };
	v2.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	v2.uvCoords = { uvX + uvWidth, uvY };

	ColorTextureVertex v3{};
	v3.position = { x, y - height,  0.0f };
	v3.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	v3.uvCoords = { uvX, uvY };

	return { v0, v1, v2, v3 };
}

//Creation
TextureQuad CreateTextureQuad(float x, float y, float width, float height, float uvX, float uvY, float uvWidth, float uvHeight) {

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
Line CreateLine(float xStart, float yStart, float xEnd, float yEnd, float stroke) {

	//Use perpendicular method
	float xScale = yEnd - yStart;
	float yScale = xEnd - xStart;
	float magnitude = invSqrt((xScale * xScale) + (yScale * yScale));
	xScale *= -1 * magnitude; yScale *= magnitude;

	ColorTextureVertex v0{};
	v0.position = { xStart - (xScale * stroke), yStart - (yScale * stroke),  LAYER_2 };
	v0.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	ColorTextureVertex v1{};
	v1.position = { xStart + (xScale * stroke), yStart + (yScale * stroke),  LAYER_2 };
	v1.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	ColorTextureVertex v2{};
	v2.position = { xEnd + (xScale * stroke), yEnd + (yScale * stroke),  LAYER_2 };
	v2.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	ColorTextureVertex v3{};
	v3.position = { xEnd - (xScale * stroke), yEnd - (yScale * stroke),  LAYER_2 };
	v3.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	return { v0, v1, v2, v3 };
}

//Creates upright equilateral triangle with approx radian values
Tri CreateTri(float x, float y, float radius) {

	ColorTextureVertex v0{};
	v0.position = { (radius * cos(1.5708f)) + x, (radius * sin(1.5708f)) + y,  LAYER_5 };
	v0.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	ColorTextureVertex v1{};
	v1.position = { (radius * cos(3.6652f)) + x, (radius * sin(3.6652f)) + y,  LAYER_5 };
	v1.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	ColorTextureVertex v2{};
	v2.position = { (radius * cos(5.7596f)) + x, (radius * sin(5.7596f)) + y,  LAYER_5 };
	v2.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	return { v0, v1, v2};
}

unsigned short int GetVerticesCount(Shape type) {
	switch (type)
	{
	//Base primitives
	case Shape::TRI:
		return Primitive::TRI_VERT;
	case Shape::QUAD:
		return Primitive::QUAD_VERT;
	case Shape::LINE:
		return Primitive::QUAD_VERT;
	default:
		return 0;
	}
}

//Colour
//Set color of single vertex
static void ColorVertex(ColorTextureVertex* vertexPointer, unsigned int vertex, float r, float g, float b) {
	vertexPointer[vertex].color.a = r;
	vertexPointer[vertex].color.b = g;
	vertexPointer[vertex].color.c = b;
}

//Set color of whole shape
void ColorShape(void* verticesArray, float r, float g, float b, Shape type)
{
	ColorTextureVertex* vertexPointer = (ColorTextureVertex*)verticesArray;

	//Set number of vertices to translate
	unsigned short int numberOfVertices = GetVerticesCount(type);

	//Color for each vertice
	for (int i = 0; i < numberOfVertices; i++) {
		ColorVertex(vertexPointer, i, r, g, b);
	}
}

//Set color of single vertex (with check for users to make sure vertex exists)
void ColorShapeVertex(void* verticesArray, unsigned int vertex, float r, float g, float b, Shape type) {
	if (vertex < GetVerticesCount(type)) {
		ColorTextureVertex* vertexPointer = (ColorTextureVertex*)verticesArray;
		ColorVertex(vertexPointer, vertex, r, g, b);
		return;
	}
	EngineLog("Error: Shape doesn't have vertice at specified location");
}

void TransparencyShape(void* verticesArray, float alpha, Shape type) {
	ColorTextureVertex* vertices = (ColorTextureVertex*)verticesArray;

	for (int i = 0; i < GetVerticesCount(type); i++) {
		vertices[i].color.d = alpha;
	}
}

void TransparencyShapeVertex(void* verticesArray, unsigned int index, float alpha, Shape type) {
	ColorTextureVertex* vertices = (ColorTextureVertex*)verticesArray;
	vertices[index].color.d = alpha;
}