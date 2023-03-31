#pragma once
#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "utility/SGUtil.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace SGRender
{
	enum class Axis
	{
		X, Y, Z
	};

	enum VProperties
	{
		V_HAS_UVS		= 0b00000001,
		V_HAS_NORMALS	= 0b00000010,
		V_HAS_COLOR		= 0b00000100,
		V_HAS_TANGENTS	= 0b00001000
	};

	//Get stride of specific property
	int PropertyStride(VProperties properties);

	//Contains the unique properties of each vertex
	enum VertexType
	{
		V_Vertex		= 0, 
		V_CVertex		= V_HAS_COLOR, 
		V_UVertex		= V_HAS_UVS, 
		V_NVertex		= V_HAS_NORMALS, 
		V_UCVertex		= V_HAS_UVS | V_HAS_COLOR, 
		V_UNVertex		= V_HAS_UVS | V_HAS_NORMALS, 
		V_NTVertex		= V_HAS_NORMALS | V_HAS_TANGENTS, 
		V_UNTVertex		= V_HAS_UVS | V_HAS_NORMALS | V_HAS_TANGENTS
	};

	int VertexStride(int vertex);
	int VertexColorOffset(int vertex);
	int vertexUVOffset(int vertex);
	int VertexNormalOffset(int vertex);
	int VertexTangentOffset(int vertex);

	struct alignas(4) Vertex;
	struct alignas(4) CVertex;
	struct alignas(4) UVertex;
	struct alignas(4) NVertex;
	struct alignas(4) UCVertex;
	struct alignas(4) UNVertex;
	struct alignas(4) NTVertex;
	struct alignas(4) UNTVertex;

	//All vertices follow ordering position -> uvs -> normals -> color -> tangent
	//Also all have a properties method to get what properties are required
	struct alignas(4) Vertex
	{
		glm::vec3 position;
	};

	struct alignas(4) CVertex
	{
		glm::vec3 position;
		glm::vec4 color;
	};

	struct alignas(4) UVertex
	{
		glm::vec3 position;
		glm::vec2 uvCoords;
	};

	struct alignas(4) NVertex
	{
		glm::vec3 position;
		glm::vec3 normals;
	};

	struct alignas(4) UCVertex
	{
		glm::vec3 position;
		glm::vec2 uvCoords;
		glm::vec4 color;
	};

	struct alignas(4) UNVertex 
	{
		glm::vec3 position;
		glm::vec2 uvCoords;
		glm::vec3 normals;
	};

	struct alignas(4) NTVertex
	{
		glm::vec3 position;
		glm::vec3 normals;
		glm::vec3 tangent;
	};

	struct alignas(4) UNTVertex
	{
		glm::vec3 position;
		glm::vec2 uvCoords;
		glm::vec3 normals;
		glm::vec3 tangent;
	};
}

#endif