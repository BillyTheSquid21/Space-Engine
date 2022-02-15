#pragma once
#ifndef OVERWORLD_H
#define OVERWORLD_H

#include "renderer/Plane.h"
#include "renderer/Texture.h"
#include "core/State.hpp"

enum class Level
{
	LEVEL_ENTRY
};

class Overworld : public State
{
public:
	void init(int width, int height, Level levelEntry);
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
	Level m_CurrentLevel = Level::LEVEL_ENTRY;

	//Test plane created
	Plane m_Plane;
	Texture m_PlaneTex;
	Quad m_Quad;

	//Input
	//Definitions for persistent input
	bool m_PersistentInput[5];
};

#define HELD_A m_PersistentInput[0]
#define HELD_D m_PersistentInput[1]
#define HELD_CTRL m_PersistentInput[2]
#define HELD_SHIFT m_PersistentInput[3]
#define HELD_W m_PersistentInput[4]

#endif