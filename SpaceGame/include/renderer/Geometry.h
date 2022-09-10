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
	const char Q_IND_COUNT = 6;

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

	//Shape translation - all shapes are defined relative to centre
	static void TranslateVertexInternal(void* vertexPointer, float deltaX, float deltaY, float deltaZ) {
		using namespace SGRender;
		Vertex* vertex = (Vertex*)(void*)vertexPointer;
		vertex->position.x += deltaX;
		vertex->position.y += deltaY;
		vertex->position.z += deltaZ;
	}

	template<typename T>
	void Translate(void* verticesArray, float deltaX, float deltaY, float deltaZ, Shape type)
	{
		T* vertexPointer = (T*)verticesArray;

		//Set number of vertices to translate
		unsigned short int numberOfVertices = GetVerticesCount(type);

		//Translate for each vertice
		for (int i = 0; i < numberOfVertices; i++) {
			TranslateVertexInternal(&vertexPointer[i], deltaX, deltaY, deltaZ);
		}
	}

	template<typename T>
	void Translate(void* verticesArray, float deltaX, float deltaY, float deltaZ, size_t verticeCount)
	{
		T* vertexPointer = (T*)verticesArray;

		//Translate for each vertice
		for (int i = 0; i < verticeCount; i++) {
			TranslateVertexInternal(&vertexPointer[i], deltaX, deltaY, deltaZ);
		}
	}

	template<typename T>
	void TranslateVertex(void* verticesArray, unsigned int index, float deltaX, float deltaY, float deltaZ)
	{
		T* vertexPointer = (T*)verticesArray;

		//Translate for each vertice
		TranslateVertexInternal(&vertexPointer[index], deltaX, deltaY, deltaZ);
	}

	//Position shapes
	template<typename T>
	void Position(void* verticesArray, Struct3f currentPosition, Struct3f newPosition, Shape type)
	{
		//get amount to translate by
		float deltaX = newPosition.a - currentPosition.a;
		float deltaY = newPosition.b - currentPosition.b;
		float deltaZ = newPosition.c - currentPosition.c;

		Translate<T>(verticesArray, deltaX, deltaY, deltaZ, type);
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

	template<typename T>
	static void SimpleScaleInternal(void* verticesArray, glm::vec3 scale, size_t verticeCount)
	{
		T* vertexPointer = (T*)verticesArray;
		for (int i = 0; i < verticeCount; i++)
		{
			vertexPointer[i].position *= scale;
		}
	}

	template<typename T>
	void SimpleScale(void* verticesArray, glm::vec3 scale, Shape type)
	{
		size_t numberOfVertices = GetVerticesCount(type);
		SimpleScaleInternal<T>(verticesArray, scale, numberOfVertices);
	}

	template<typename T>
	void SimpleScale(void* verticesArray, glm::vec3 scale, size_t verticeCount)
	{
		SimpleScaleInternal<T>(verticesArray, scale, verticeCount);
	}
}

#endif