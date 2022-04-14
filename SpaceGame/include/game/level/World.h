#pragma once
#ifndef WORLD_H
#define WORLD_H

#include "renderer/Plane.h"
#include "renderer/Texture.h"
#include "game/objects/TileMap.h"
#include <stdint.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidxml/rapidxml.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <atomic>

//Tiles
namespace World
{
	//Pixels in one tile
	const float TILE_SIZE = 32.0f;

	//Level data
	enum class LevelID
	{
		LEVEL_ENTRY, LEVEL_TEST,

		//Null entry - keep at end for size purposes
		LEVEL_NULL
	};

	//When level data is written, use int value the respective enum value evaluates to

	//Directions - north is -Z axis, south is +Z axis, west is +X axis, east is -X (taken from abs)
	enum class Direction : uint8_t
	{
		DIRECTION_NULL,
		NORTH, NORTHEAST, NORTHWEST,	//Quads are aligned with vertex 0 and 1 being north facing
		SOUTH, SOUTHEAST, SOUTHWEST,	//Each level is TILE_SIZE / sqrt(2) up due to geometry
		EAST,
		WEST
	};

	World::Direction GetDirection(std::string dir);

	enum class WorldLevel : int
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
		//Level bridge - lets walk into next level so don't need to check outside bounds
		LEVEL_BRIDGE,
		//Sprite blocking
		SPRITE_BLOCKING,
	};

	//Tile arranging classes
	void tileLevel(TextureQuad* quad, WorldLevel level);
	void SlopeTile(TextureQuad* quad, Direction direction);

	struct TileTexture
	{
		unsigned int textureX;
		unsigned int textureY;
	};

	struct TileLoc
	{
		unsigned int x;
		unsigned int z;
	};

	struct LevelPermission
	{
		MovementPermissions perm;
		bool leaving = false;
	};

	TileLoc NextTileInInputDirection(Direction direct, TileLoc tile);
	LevelPermission RetrievePermission(World::LevelID level, World::Direction direction, World::TileLoc loc);
	LevelPermission RetrievePermission(World::LevelID level, World::TileLoc loc);
	void ModifyTilePerm(World::LevelID level, World::Direction direction, World::TileLoc loc);

	struct LevelDimensions
	{
		unsigned int levelW;
		unsigned int levelH;
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
		std::vector<MovementPermissions> planePermissions;
		std::vector<TileTexture> planeTextures;
	};

	LevelData ParseLevel(World::LevelID id);

	class Level
	{
	public:
		//Load level data
		bool buildLevel(Render::Renderer<TextureVertex>* planeRenderer, TileMap* tileMapPointer);
		void setID(World::LevelID id) { m_ID = id; }
		void purgeLevel(); //Clears heap data
		void render();
		bool loaded() const { return m_Loaded; }
		void setLoaded(bool loaded) { m_Loaded = loaded; }

		//Static methods
		static Struct2f queryOrigin(LevelID level) { if (s_LevelOriginCache.find(level) != s_LevelOriginCache.end()) { return s_LevelOriginCache[level]; } EngineLog("Origin not found!"); return s_LevelOriginCache.begin()->second; } //If fails, return first level origin found
		static LevelDimensions queryDimensions(LevelID level) { if (s_LevelDimensionCache.find(level) != s_LevelDimensionCache.end()) { return s_LevelDimensionCache[level]; } return s_LevelDimensionCache.begin()->second; } //If fails, return first level dim found
		static std::vector<MovementPermissions>* queryPermissions(LevelID level) { if (s_MovementPermissionsCache.find(level) != s_MovementPermissionsCache.end()) { return s_MovementPermissionsCache[level]; } return s_MovementPermissionsCache.begin()->second; } //If fails, return first level permis found
		
		//Static caches
		static std::unordered_map<LevelID, Struct2f> s_LevelOriginCache;
		static std::unordered_map<LevelID, LevelDimensions> s_LevelDimensionCache;
		static std::unordered_map<LevelID, std::vector<MovementPermissions>*> s_MovementPermissionsCache;
	private:
		//ID
		LevelID m_ID = World::LevelID::LEVEL_NULL;

		//All levels have a rendered plane and a grid of tiles
		Plane m_Plane;
		std::vector<MovementPermissions> m_Permissions;
		std::vector<WorldLevel> m_Heights;
		TileMap* m_TileMapPointer;

		unsigned int m_LevelTilesX = 0;
		unsigned int m_LevelTilesY = 0;
		unsigned int m_LevelTotalTiles = 0;


		float m_XOffset = 0.0f; float m_YOffset = 0.0f;
		bool m_Loaded = false;
	};

	//Constants
	constexpr float WALK_SPEED = 1.0f;
	constexpr float RUN_SPEED = 3.1f;
	constexpr float WALK_DURATION = 1.0f / WALK_SPEED;
	constexpr float RUN_DURATION = 1.0f / RUN_SPEED;
	constexpr float WALK_FRAME_DURATION = WALK_DURATION / 4.0f;
	constexpr float RUN_FRAME_DURATION = RUN_DURATION / 4.0f;
}

#endif