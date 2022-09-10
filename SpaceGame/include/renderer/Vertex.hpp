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

	//alignas used to confirm all bytes are contiguous in memory - will be reworked if needed
	struct alignas(4) Vertex
	{
		glm::vec3 position;
	};

	struct alignas(4) CVertex : public Vertex
	{
		glm::vec4 color;
	};

	struct alignas(4) TVertex : public Vertex
	{
		glm::vec2 uvCoords;
	};

	struct alignas(4) CTVertex : public TVertex
	{
		glm::vec4 color;
	};

	struct alignas(4) NTVertex : public TVertex
	{
		glm::vec3 normals;
	};
}

#endif