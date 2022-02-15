#pragma once
#ifndef OVERWORLD_H
#define OVERWORLD_H

#include "renderer/Plane.h"
#include "renderer/Texture.h"
#include "core/State.hpp"

//Tiles
namespace World 
{
	//Pixels in one tile
	const float TILE_SIZE = 32.0f;

	//Level data
	enum class LevelID
	{
		LEVEL_ENTRY
	};

	//Directions - north is +Z axis, south is -Z axis, west is +X axis, east is -X 
	enum class Direction
	{
		NORTH, NORTHEAST, NORTHWEST,	//Quads are aligned with vertex 0 and 1 being north facing
		SOUTH, SOUTHEAST, SOUTHWEST,	//Each level is TILE_SIZE / sqrt(2) up due to geometry
		EAST, 
		WEST,
	};

	enum class WorldLevel
	{
		//Above ground
		F0 = 0, F1 = 1, F2 = 2, F3 = 3, F4 = 4, F5 = 5, 
		F6 = 6, F7 = 7, F8 = 8, F9 = 9, F10 = 10,
		//Below ground
		B1 = -1, B2 = -2, B3 = -3, B4 = -4, B5 = -5, 
		B6 = -6, B7 = -7, B8 = -8, B9 = - 9, B10 = -10
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
	void tileLevel(Quad* quad, WorldLevel level);
	void SlopeTile(Quad* quad, Direction direction);

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

class Overworld : public State
{
public:
	void init(int width, int height, World::LevelID levelEntry);
	void update(double deltaTime, double time);
	void render();
	void loadRequiredData();
	void purgeRequiredData();
	void handleInput(int key, int scancode, int action, int mods);

private:
	Renderer<Vertex> m_Renderer;
	Texture m_PlaneTexture;
	Shader m_Shader;
	Camera m_Camera;
	int m_Width; int m_Height;

	//Current level - defualts to the entry level
	World::LevelID m_CurrentLevel = World::LevelID::LEVEL_ENTRY;

	//Test plane created
	Plane m_Plane;
	Texture m_PlaneTex;

	//Input
	//Definitions for persistent input
	bool m_PersistentInput[8];
};

//Definitions for inputs that persist
#define HELD_A m_PersistentInput[0]
#define HELD_D m_PersistentInput[1]
#define HELD_CTRL m_PersistentInput[2]
#define HELD_SHIFT m_PersistentInput[3]
#define HELD_Q m_PersistentInput[4]
#define HELD_E m_PersistentInput[5]
#define HELD_W m_PersistentInput[6]
#define HELD_S m_PersistentInput[7]

#endif