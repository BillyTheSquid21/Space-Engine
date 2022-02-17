#pragma once
#ifndef WORLD_H
#define WORLD_H

#include "renderer/Plane.h"
#include "renderer/Texture.h"

//Tiles
namespace World
{
	//Pixels in one tile
	const float TILE_SIZE = 32.0f;

	//Level data
	enum class LevelID
	{
		LEVEL_ENTRY,

		//Null entry
		LEVEL_NULL
	};

	//Directions - north is +Z axis, south is -Z axis, west is +X axis, east is -X 
	enum class Direction
	{
		NORTH, NORTHEAST, NORTHWEST,	//Quads are aligned with vertex 0 and 1 being north facing
		SOUTH, SOUTHEAST, SOUTHWEST,	//Each level is TILE_SIZE / sqrt(2) up due to geometry
		EAST,
		WEST,
		DIRECTION_NULL
	};

	enum class WorldLevel
	{
		//Above ground
		F0 = 0, F1 = 1, F2 = 2, F3 = 3, F4 = 4, F5 = 5,
		F6 = 6, F7 = 7, F8 = 8, F9 = 9, F10 = 10,
		//Below ground
		B1 = -1, B2 = -2, B3 = -3, B4 = -4, B5 = -5,
		B6 = -6, B7 = -7, B8 = -8, B9 = -9, B10 = -10
	};

	enum class MovementPermissions
	{
		//Basic permission
		CLEAR, WALL,
		//Slopes - changes character y pos as moves through
		STAIRS_NORTH, STAIRS_SOUTH, STAIRS_EAST, STAIRS_WEST,
		//Terrain
		WATER, LEDGE_SOUTH, LEDGE_NORTH, LEDGE_EAST, LEDGE_WEST,
	};

	//Tile arranging classes
	void tileLevel(TextureQuad* quad, WorldLevel level);
	void SlopeTile(TextureQuad* quad, Direction direction);

	struct Tile
	{
		//location in plane
		unsigned int xPlane;
		unsigned int yPlane;

		//Position in world
		int xPos;
		int yPos;

		//Height
		WorldLevel level;
		MovementPermissions permissions;
	};

	struct LevelData
	{
		//DATA HERE
	};

	class Level
	{
	public:
		~Level() { delete[] tiles; }

	private:
		//ID
		LevelID code;

		//All levels have a rendered plane and a grid of tiles
		Plane plane;
		Tile* tiles;
	};
}

#endif