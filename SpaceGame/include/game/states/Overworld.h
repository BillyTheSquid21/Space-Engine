#pragma once
#ifndef OVERWORLD_H
#define OVERWORLD_H

#include "game/level/World.h"
#include "game/objects/OWObjects.h"
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
	Renderer<TextureVertex> m_Renderer;
	Texture m_PlaneTexture;
	Shader m_Shader;
	Camera m_Camera;
	int m_Width; int m_Height;

	//Current level - defualts to the entry level
	World::LevelID m_CurrentLevel = World::LevelID::LEVEL_ENTRY;

	//Test plane created
	Plane m_Plane;
	Texture m_PlaneTex;

	//Test obj management
	ObjectManager m_Manager;

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