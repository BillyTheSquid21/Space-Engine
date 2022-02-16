#pragma once
#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "SGUtil.h"

enum class Axis
{
	X, Y, Z
};

struct alignas(4) VertexBase
{
	Component3f position;
};

struct alignas(4) Vertex : public VertexBase
{
	Component4f color;
	Component2f uvCoords;
};

struct alignas(4) TextureVertex : public VertexBase
{
	Component2f uvCoords;
};

#endif