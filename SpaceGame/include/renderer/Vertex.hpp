#pragma once
#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "SGUtil.h"

enum class Axis
{
	X, Y, Z
};

struct alignas(4) Vertex
{
	Component3f position;
	Component4f color;
	Component2f uvCoords;
};

#endif