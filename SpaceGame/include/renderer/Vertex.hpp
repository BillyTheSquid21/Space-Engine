#pragma once
#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "utility/SGUtil.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

enum class Axis
{
	X, Y, Z
};

//alignas used to confirm all bytes are contiguous in memory - will be reworked if needed
struct alignas(4) Vertex
{
	glm::vec3 position;
};

struct alignas(4) TextureVertex : public Vertex
{
	glm::vec2 uvCoords;
};

struct alignas(4) ColorTextureVertex : public TextureVertex
{
	glm::vec4 color;
};

struct alignas(4) NormalTextureVertex : public TextureVertex
{
	glm::vec3 normals;
};

#endif