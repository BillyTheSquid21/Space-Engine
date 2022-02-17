#pragma once
#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "SGUtil.h"

enum class Axis
{
	X, Y, Z
};

//alignas used to confirm all bytes are contiguous in memory - will be reworked if needed
struct alignas(4) Vertex
{
	Component3f position;
};

struct alignas(4) ColorTextureVertex : public Vertex
{
	Component4f color;
	Component2f uvCoords;
};

struct alignas(4) TextureVertex : public Vertex
{
	Component2f uvCoords;
};

#endif