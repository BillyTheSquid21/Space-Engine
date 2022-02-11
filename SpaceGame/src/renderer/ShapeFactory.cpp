#include "renderer/ShapeFactory.h"

//Creation
Quad CreateQuad(float x, float y, float width, float height, float uvX, float uvY, float uvWidth, float uvHeight) {

	Vertex v0{};
	v0.position = { x, y , 0.0f };
	v0.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	v0.uvCoords = { uvX, uvY + uvHeight };

	Vertex v1{};
	v1.position = { x + width, y,  0.0f };
	v1.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	v1.uvCoords = { uvX + uvWidth, uvY + uvHeight };

	Vertex v2{};
	v2.position = { x + width, y - height,  0.0f };
	v2.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	v2.uvCoords = { uvX + uvWidth, uvY };

	Vertex v3{};
	v3.position = { x, y - height,  0.0f };
	v3.color = { 1.0f, 1.0f, 1.0f, 1.0f };
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

	Vertex v0{};
	v0.position = { xStart - (xScale * stroke), yStart - (yScale * stroke),  LAYER_2 };
	v0.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	Vertex v1{};
	v1.position = { xStart + (xScale * stroke), yStart + (yScale * stroke),  LAYER_2 };
	v1.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	Vertex v2{};
	v2.position = { xEnd + (xScale * stroke), yEnd + (yScale * stroke),  LAYER_2 };
	v2.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	Vertex v3{};
	v3.position = { xEnd - (xScale * stroke), yEnd - (yScale * stroke),  LAYER_2 };
	v3.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	return { v0, v1, v2, v3 };
}

//Creates upright equilateral triangle with approx radian values
Tri CreateTri(float x, float y, float radius) {

	Vertex v0{};
	v0.position = { (radius * cos(1.5708f)) + x, (radius * sin(1.5708f)) + y,  LAYER_5 };
	v0.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	Vertex v1{};
	v1.position = { (radius * cos(3.6652f)) + x, (radius * sin(3.6652f)) + y,  LAYER_5 };
	v1.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	Vertex v2{};
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

unsigned short int GetFloatCount(Shape type) {
	return (sizeof(Vertex) / sizeof(float)) * GetVerticesCount(type);
}

//Translation
void TranslateShape(void* verticesArray, float deltaX, float deltaY, float deltaZ, Shape type) 
{	
	Vertex* vertexPointer = (Vertex*)verticesArray;

	//Set number of vertices to translate
	unsigned short int numberOfVertices = GetVerticesCount(type);

	//Translate for each vertice
	for (int i = 0; i < numberOfVertices; i++) {
		vertexPointer[i].position.a += deltaX;
		vertexPointer[i].position.b += deltaY;
		vertexPointer[i].position.c += deltaZ;
	}
}

//Position
void PositionShape(void* verticesArray, Component3f currentPosition, Component3f newPosition, Shape type)
{
	//get amount to translate by
	float deltaX = newPosition.a - currentPosition.a;
	float deltaY = newPosition.b - currentPosition.b;
	float deltaZ = newPosition.c - currentPosition.c;

	TranslateShape(verticesArray, deltaX, deltaY, deltaZ, type);
}

//Rotation
void RotateShape(void* verticesArray, Component3f rotationCentre, float angle, Shape type, Axis axis) 
{
	Vertex* vertexPointer = (Vertex*)verticesArray;

	//Set number of vertices to translate
	unsigned short int numberOfVertices = GetVerticesCount(type);

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
	for (int i = 0; i < numberOfVertices; i++) {
		glm::vec3 position = { vertexPointer[i].position.a - rotationCentre.a, vertexPointer[i].position.b - rotationCentre.b, vertexPointer[i].position.c - rotationCentre.c };
		position = glm::rotate(position, glm::radians(angle), axisVector);
		vertexPointer[i].position = { position.x + rotationCentre.a, position.y + rotationCentre.b, position.z + rotationCentre.c };
	}
}

//put on layer - stick to whole numbers personally to save confusion
void LayerShape(void* verticesArray, float layer, Shape type) {
	Vertex* vertexPointer = (Vertex*)verticesArray;

	//Set number of vertices to translate
	unsigned short int numberOfVertices = GetVerticesCount(type);

	//Translate for each vertice
	for (int i = 0; i < numberOfVertices; i++) {
		vertexPointer[i].position.c = layer;
	}
}

//Color

//Set color of single vertex
static void ColorVertex(Vertex* vertexPointer, unsigned int vertex, float r, float g, float b) {
	vertexPointer[vertex].color.a = r;
	vertexPointer[vertex].color.b = g;
	vertexPointer[vertex].color.c = b;
}

//Set color of whole shape
void ColorShape(void* verticesArray, float r, float g, float b, Shape type)
{
	Vertex* vertexPointer = (Vertex*)verticesArray;

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
		Vertex* vertexPointer = (Vertex*)verticesArray;
		ColorVertex(vertexPointer, vertex, r, g, b);
		return;
	}
	EngineLog("Error: Shape doesn't have vertice at specified location");
}

void TransparencyShape(void* verticesArray, float alpha, Shape type) {
	Vertex* vertices = (Vertex*)verticesArray;

	for (int i = 0; i < GetVerticesCount(type); i++) {
		vertices[i].color.d = alpha;
	}
}

void TransparencyShapeVertex(void* verticesArray, unsigned int index, float alpha, Shape type) {
	Vertex* vertices = (Vertex*)verticesArray;
	vertices[index].color.d = alpha;
}