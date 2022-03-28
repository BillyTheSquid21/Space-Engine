#pragma once
#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "utility/SGUtil.h"

enum class Axis
{
	X, Y, Z
};

//alignas used to confirm all bytes are contiguous in memory - will be reworked if needed
struct alignas(4) Vertex
{
	Struct3f position;
};

struct alignas(4) TextureVertex : public Vertex
{
	Struct2f uvCoords;
};

struct alignas(4) ColorTextureVertex : public TextureVertex
{
	Struct4f color;
};

#endif