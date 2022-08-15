#pragma once
#ifndef WORLD_H
#define WORLD_H

#include "fstream"
#include "string"
#include "sstream"
#include "atomic"
#include "stdint.h"

#include "renderer/Plane.h"
#include "renderer/Texture.h"
#include "game/objects/TileMap.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidxml/rapidxml.hpp"

//Tiles
namespace World
{
	//Pixels in one tile
	const float TILE_SIZE = 32.0f;

	//Level data
	enum class LevelID : unsigned int
	{
		LEVEL_ENTRY = 0, LEVEL_TEST = 1,

		//Null entry - keep at end for size purposes
		LEVEL_NULL = 10000
	};

	//When level data is written, use int value the respective enum value evaluates to

	//Slope Directions - north is -Z axis, south is +Z axis, west is +X axis, east is -X (taken from abs)
	enum class Direction : uint8_t
	{
		DIRECTION_NULL,
		NORTH, NORTHEAST, NORTHWEST,	//Quads are aligned with vertex 0 and 1 being north facing
		SOUTH, SOUTHEAST, SOUTHWEST,	//Each level is TILE_SIZE / sqrt(2) up due to geometry
		EAST,
		WEST,
		NORTHEAST_WRAPPED, NORTHWEST_WRAPPED, 
		SOUTHEAST_WRAPPED, SOUTHWEST_WRAPPED //Wraps slope around corner
	};

	World::Direction GetDirection(std::string dir);

	enum class WorldHeight : int8_t //Has been previously known as "WorldLevel" - changed because of confusion with levelID
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
		CLEAR = 0, WALL = 1,
		//Slopes - changes character y pos as moves through - must be on both levels
		STAIRS_NORTH = 2, STAIRS_SOUTH = 3, STAIRS_EAST = 4, STAIRS_WEST = 5,
		//Terrain
		WATER = 6, LEDGE_NORTH = 7, LEDGE_SOUTH = 8, LEDGE_EAST = 9, LEDGE_WEST = 10,
		//Level bridge - lets walk into next level so don't need to check outside bounds
		LEVEL_BRIDGE = 11,
		//Sprite blocking
		SPRITE_BLOCKING = 12,
		//Encounter
		TALL_GRASS = 13, CAVE = 14,
	};

	//Tile arranging classes
	void tileLevel(Norm_Tex_Quad* quad, WorldHeight level);
	void SlopeTile(Norm_Tex_Quad* quad, Direction direction);

	struct TileTexture
	{
		unsigned int textureX;
		unsigned int textureY;
	};

	struct Tile
	{
		int x;
		int z;

		bool operator==(Tile& tile) { return (this->x == tile.x && this->z == tile.z); };
		Tile operator-(Tile& tile) { Tile newTile; newTile.x = this->x - tile.x; newTile.z = this->z - tile.z; return newTile; }
		Tile operator+(Tile& tile) { Tile newTile; newTile.x = this->x + tile.x; newTile.z = this->z + tile.z; return newTile; }
	};

	struct LevelPermission
	{
		MovementPermissions perm;
		MovementPermissions* permPointer;
		bool leaving = false;
	};

	Tile NextTileInDirection(Direction direct, Tile tile);
	World::Direction DirectionOfAdjacentTile(World::Tile scr, World::Tile dest);
	LevelPermission RetrievePermission(World::LevelID level, World::Direction direction, World::Tile loc, WorldHeight height);
	LevelPermission RetrievePermission(World::LevelID level, World::Tile loc, WorldHeight height);
	void ModifyTilePerm(World::LevelID level, World::Direction direction, World::Tile loc, WorldHeight height, MovementPermissions& lastPerm, MovementPermissions*& lastPermPtr);
	MovementPermissions* GetTilePermission(World::LevelID level, World::Tile loc, WorldHeight height);

	bool CheckPlayerInteracting(Tile player, Tile script, Direction playerFacing);
	Direction OppositeDirection(Direction dir);

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
		std::vector<WorldHeight> planeHeights;
		std::vector<WorldHeight> presentWorldLevels;
		std::vector<Direction> planeDirections;
		std::vector<MovementPermissions> planePermissions;
		std::vector<TileTexture> planeTextures;
	};

	LevelData ParseLevel(World::LevelID id);

	class Level
	{
	public:
		//Load level data
		bool buildLevel(Render::Renderer<NormalTextureVertex>* planeRenderer, TileMap* tileMapPointer);
		void setID(World::LevelID id) { m_ID = id; }
		void purgeLevel(); //Clears heap data
		void render();
		bool loaded() const { return m_Loaded; }
		void setLoaded(bool loaded) { m_Loaded = loaded; }

		//Static methods
		static Struct2f queryOrigin(LevelID level) { if ((unsigned int)level < s_LevelOriginCache.size()) { return s_LevelOriginCache[(unsigned int)level]; } EngineLog("Origin not found!"); return s_LevelOriginCache[0]; } //If fails, return first level origin found
		static LevelDimensions queryDimensions(LevelID level) { if ((unsigned int)level < s_LevelDimensionCache.size()) { return s_LevelDimensionCache[(unsigned int)level]; } return s_LevelDimensionCache[0]; } //If fails, return first level dim found
		
		//Allows returning a portion of the permission vector based on height
		struct PermVectorFragment
		{
			MovementPermissions* pointer;
			size_t size;
		};
		
		static PermVectorFragment queryPermissions(LevelID level, WorldHeight height);
		static std::vector<MovementPermissions>* getPermissions(LevelID level);
		//Static caches - for data that won't change at runtime
		static std::vector<Struct2f> s_LevelOriginCache;
		static std::vector<LevelDimensions> s_LevelDimensionCache;
		static bool s_CacheInit;

		//Stores pointers to important data within level
		struct LevelPtrCache
		{
			std::vector<MovementPermissions>* perms = nullptr;
			std::vector<WorldHeight>* levels = nullptr;
		};
		
		//Static caches - for data that needs to be accessed at source as can change at runtime
		static std::vector<LevelPtrCache> s_MovementPermissionsCache;

	private:
		//ID
		LevelID m_ID = World::LevelID::LEVEL_NULL;

		//All levels have a rendered plane and a grid of tiles
		Plane m_Plane;
		std::vector<WorldHeight> m_AvailibleLevels;
		std::vector<MovementPermissions> m_Permissions;
		std::vector<WorldHeight> m_Heights;
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