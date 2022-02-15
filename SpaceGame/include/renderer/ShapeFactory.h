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
#include "SGUtil.h"
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

#define Quad std::array<Vertex, 4>
#define Line std::array<Vertex, 4>	//Line is just a quad set up to be more convinient
#define Tri std::array<Vertex, 3>

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
Line CreateLine(float xStart, float yStart, float xEnd, float yEnd, float stroke);
Tri CreateTri(float x, float y, float radius);

//Shape translation - all shapes are defined relative to centre
void TranslateShape(void* verticesArray, float deltaX, float deltaY, float deltaZ, Shape type);
void TranslateShapeVertex(void* verticesArray, unsigned int index, float deltaX, float deltaY, float deltaZ);
void PositionShape(void* verticesArray, Component3f currentPosition, Component3f newPosition, Shape type);
void RotateShape(void* verticesArray, Component3f rotationCentre, float angle, Shape type, Axis axis);
void ColorShape(void* verticesArray, float r, float g, float b, Shape type);
void ColorShapeVertex(void* verticesArray, unsigned int vertex, float r, float g, float b, Shape type);
void LayerShape(void* verticesArray, float layer, Shape type);
void TransparencyShape(void* verticesArray, float alpha, Shape type);
void TransparencyShapeVertex(void* verticesArray, unsigned int index, float alpha, Shape type);

//Utility
unsigned short int GetVerticesCount(Shape type);
unsigned short int GetFloatCount(Shape type);

#endif
