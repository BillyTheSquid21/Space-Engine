#pragma once
#ifndef SHAPE_FACTORY_H
#define SHAPE_FACTORY_H

#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
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
	const char Q_IND_COUNT = 6;

	//Tris
	const char TRI_VERT = 3;
	const unsigned int T_IND[6]
	{
		0, 1, 2
	};	
	const char T_IND_COUNT = 3;
}

#define Quad std::array<ColorTextureVertex, 4>
#define TextureQuad std::array<TextureVertex, 4>
#define Line std::array<ColorTextureVertex, 4>	//Line is just a quad set up to be more convinient
#define Tri std::array<ColorTextureVertex, 3>


enum class Shape
{	
	//Base shapes
	TRI, QUAD, LINE
};

//Layer constants - not necessary but makes easier and provides enough layers
const float LAYER_1 = -0.8f;
const float LAYER_2 = -0.6f;
const float LAYER_3 = -0.4f;
const float LAYER_4 = -0.2f;
const float LAYER_5 =  0.0f;
const float LAYER_6 =  0.2f;
const float LAYER_7 =  0.4f;
const float LAYER_8 =  0.6f;
const float LAYER_9 =  0.8f;
const float GUI_LAYER_1 = 0.82f;
const float GUI_LAYER_2 = 0.84f;
const float GUI_LAYER_3 = 0.86f;
const float GUI_LAYER_4 = 0.88f;
const float GUI_LAYER_5 = 0.90f;
const float GUI_LAYER_6 = 0.92f;

//Shape Creation
Quad CreateQuad(float x, float y, float width, float height, float uvX, float uvY, float uvWidth, float uvHeight);
TextureQuad CreateTextureQuad(float x, float y, float width, float height, float uvX, float uvY, float uvWidth, float uvHeight);
Line CreateLine(float xStart, float yStart, float xEnd, float yEnd, float stroke);
Tri CreateTri(float x, float y, float radius);

//Currently only applies to color texture vertex - will update if another derived vertex needs to access colour
void ColorShape(void* verticesArray, float r, float g, float b, Shape type);
void ColorShapeVertex(void* verticesArray, unsigned int vertex, float r, float g, float b, Shape type);
void TransparencyShape(void* verticesArray, float alpha, Shape type);
void TransparencyShapeVertex(void* verticesArray, unsigned int index, float alpha, Shape type);

//Currently only applies to texture vertex - will update if another derived vertex needs to access uvs
void SetQuadUV(TextureVertex* verticesArray, float u, float v, float width, float height);

//Utility
unsigned short int GetVerticesCount(Shape type);
template<typename T>
unsigned short int GetFloatCount(Shape type) {
	return (sizeof(T) / sizeof(float)) * GetVerticesCount(type);
}

//Shape translation - all shapes are defined relative to centre
static void TranslateShapeVertexInternal(void* vertexPointer, float deltaX, float deltaY, float deltaZ) {
	Vertex* vertex = (Vertex*)(void*)vertexPointer;
	vertex->position.a += deltaX;
	vertex->position.b += deltaY;
	vertex->position.c += deltaZ;
}

template<typename T>
void TranslateShape(void* verticesArray, float deltaX, float deltaY, float deltaZ, Shape type)
{
	T* vertexPointer = (T*)verticesArray;

	//Set number of vertices to translate
	unsigned short int numberOfVertices = GetVerticesCount(type);

	//Translate for each vertice
	for (int i = 0; i < numberOfVertices; i++) {
		TranslateShapeVertexInternal(&vertexPointer[i], deltaX, deltaY, deltaZ);
	}
}

template<typename T>
void TranslateShapeVertex(void* verticesArray, unsigned int index, float deltaX, float deltaY, float deltaZ)
{
	T* vertexPointer = (T*)verticesArray;

	//Translate for each vertice
	TranslateShapeVertexInternal(&vertexPointer[index], deltaX, deltaY, deltaZ);
}

//Position shapes
template<typename T>
void PositionShape(void* verticesArray, Struct3f currentPosition, Struct3f newPosition, Shape type)
{
	//get amount to translate by
	float deltaX = newPosition.a - currentPosition.a;
	float deltaY = newPosition.b - currentPosition.b;
	float deltaZ = newPosition.c - currentPosition.c;

	TranslateShape<T>(verticesArray, deltaX, deltaY, deltaZ, type);
}

//Rotation
template<typename T>
void RotateShape(void* verticesArray, Struct3f rotationCentre, float angle, Shape type, Axis axis)
{
	T* vertexPointer = (T*)verticesArray;

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
		Vertex* vertex = (Vertex*)(void*)&vertexPointer[i];
		glm::vec3 position = { vertex->position.a - rotationCentre.a, vertex->position.b - rotationCentre.b, vertex->position.c - rotationCentre.c };
		position = glm::rotate(position, glm::radians(angle), axisVector);
		vertex->position = { position.x + rotationCentre.a, position.y + rotationCentre.b, position.z + rotationCentre.c };
	}
}

#endif
