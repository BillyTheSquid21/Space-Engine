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

	//All vertices follow ordering position -> uvs -> normals -> color -> tangent
	//Also all have a properties method to get what properties are required
	struct alignas(4) Vertex
	{
		glm::vec3 position;
		static int properties() { return 0; }
		static int stride() { return 3; }
	};

	struct alignas(4) CVertex
	{
		glm::vec3 position;
		glm::vec4 color;
		static int properties() { return V_HAS_COLOR; }
		static int stride() { return 7; }
		static int colorOffset() { return 3; }
	};

	struct alignas(4) UVertex
	{
		glm::vec3 position;
		glm::vec2 uvCoords;
		static int properties() { return V_HAS_UVS; }
		static int stride() { return 5; }
		static int uvOffset() { return 3; }
	};

	struct alignas(4) NVertex
	{
		glm::vec3 position;
		glm::vec3 normals;
		static int properties() { return V_HAS_NORMALS; }
		static int stride() { return 6; }
		static int normalOffset() { return 3; }
	};

	struct alignas(4) UCVertex
	{
		glm::vec3 position;
		glm::vec2 uvCoords;
		glm::vec4 color;
		static int properties() { return V_HAS_UVS | V_HAS_COLOR; }
		static int stride() { return 9; }
		static int colorOffset() { return 5; }
		static int uvOffset() { return 3; }
	};

	struct alignas(4) UNVertex 
	{
		glm::vec3 position;
		glm::vec2 uvCoords;
		glm::vec3 normals;
		static int properties() { return V_HAS_UVS | V_HAS_NORMALS; }
		static int stride() { return 8; }
		static int normalOffset() { return 5; }
		static int uvOffset() { return 3; }
	};

	struct alignas(4) NTVertex
	{
		glm::vec3 position;
		glm::vec3 normals;
		glm::vec3 tangent;
		static int properties() { return V_HAS_TANGENTS | V_HAS_NORMALS; }
		static int stride() { return 9; }
		static int normalOffset() { return 3; }
		static int tangentOffset() { return 6; }
	};

	struct alignas(4) UNTVertex
	{
		glm::vec3 position;
		glm::vec2 uvCoords;
		glm::vec3 normals;
		glm::vec3 tangent;
		static int properties() { return V_HAS_UVS | V_HAS_NORMALS | V_HAS_TANGENTS; }
		static int stride() { return 11; }
		static int normalOffset() { return 5; }
		static int uvOffset() { return 3; }
		static int tangentOffset() { return 8; }
	};
}

#endif