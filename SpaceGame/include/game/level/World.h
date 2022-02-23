#pragma once
#ifndef WORLD_H
#define WORLD_H

#include "renderer/Plane.h"
#include "renderer/Texture.h"
#include "game/objects/TileMap.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <string>
#include <sstream>

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

	//When level data is written, use int value the respective enum value evaluates to

	//Directions - north is +Z axis, south is -Z axis, west is +X axis, east is -X 
	enum class Direction
	{
		DIRECTION_NULL,
		NORTH, NORTHEAST, NORTHWEST,	//Quads are aligned with vertex 0 and 1 being north facing
		SOUTH, SOUTHEAST, SOUTHWEST,	//Each level is TILE_SIZE / sqrt(2) up due to geometry
		EAST,
		WEST
	};

	enum class WorldLevel
	{
		//Above ground
		F0 = 0, F1 = 1, F2 = 2, F3 = 3, F4 = 4, F5 = 5,
		F6 = 6, F7 = 7, F8 = 8, F9 = 9, F10 = 10,
		//Below ground
		B1 = -1, B2 = -2, B3 = -3, B4 = -4, B5 = -5,
		B6 = -6, B7 = -7, B8 = -8, B9 = -9, B10 = -10,
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

	struct TileTexture
	{
		unsigned int textureX;
		unsigned int textureY;
	};

	struct LevelData
	{
		//DATA HERE
		LevelID id;
		unsigned int width;
		unsigned int height;
		float originX;
		float originY;
		std::vector<WorldLevel> planeHeights;
		std::vector<Direction> planeDirections;
		std::vector<TileTexture> planeTextures;
	};

	LevelData ParseLevel();

	class Level
	{
	public:
		//Load level data
		void buildLevel(unsigned int tilesX, unsigned int tilesY, Renderer<TextureVertex>* planeRenderer, TileMap* tileMapPointer);
		void render();
		static Component2f queryOrigin(LevelID level) { if (s_LevelOriginCache.find(level) != s_LevelOriginCache.end()) { return s_LevelOriginCache[level]; } return s_LevelOriginCache.begin()->second; } //If fails, return first level origin found
		static std::unordered_map<LevelID, Component2f> s_LevelOriginCache;
	private:
		//ID
		LevelID code;

		//All levels have a rendered plane and a grid of tiles
		Plane plane;
		std::vector<MovementPermissions> m_Permissions;
		std::vector<WorldLevel> m_Heights;
		TileMap* m_TileMapPointer;

		unsigned int m_LevelTilesX = 0;
		unsigned int m_LevelTilesY = 0;
		unsigned int m_LevelTotalTiles = 0;


		float m_XOffset = 0.0f; float m_YOffset = 0.0f;
	};

}

#endif