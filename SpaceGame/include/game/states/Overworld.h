#pragma once
#ifndef OVERWORLD_H
#define OVERWORLD_H

#include "game/level/World.h"
#include "game/objects/OWObjects.h"
#include "game/objects/TileMap.h"
#include "core/State.hpp"
#include "core/ObjManagement.h"

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
	//Rendering
	Renderer<TextureVertex> m_WorldRenderer;
	Renderer<TextureVertex> m_SpriteRenderer;
	Shader m_Shader;
	Camera m_Camera;
	int m_Width; int m_Height;

	//Current level - defualts to the entry level
	World::LevelID m_CurrentLevel = World::LevelID::LEVEL_ENTRY;

	//Textures
	Texture m_PlaneTexture;
	Texture m_OWSprites;

	//Test plane
	Plane m_Plane;

	//Test level
	World::Level level;

	//Test obj mng
	ObjectManager m_ObjManager;

	//Test tile mapping
	TileMap m_OverworldTileMap = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	TileMap m_SpriteTileMap = TileMap(640.0f, 320.0f, 32.0f, 32.0f);

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