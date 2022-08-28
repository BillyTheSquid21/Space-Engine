#pragma once
#ifndef OBJECT_TYPES_HPP
#define OBJECT_TYPES_HPP

#include "stdint.h"

enum class ObjectType : uint16_t
{
	Trees, Grass, DirectionalSprite, WalkingSprite, RunningSprite, Bridge, LoadingZone, WarpTile, ScriptTile, Model, NULL_TYPE
};

#endif