#pragma once
#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "renderer/Renderer.hpp"
#include "renderer/Texture.h"
#include "core/State.hpp"
#include "game/gui/GUI.h"
#include "core/Game.h"

class MainMenu : public State
{
public:
	void init(int width, int height, GLFWwindow* window);
	void update(double deltaTime, double time);
	void render();
private:
	GLFWwindow* m_WindowPointer;
	Renderer<Vertex> m_Renderer;
	Shader m_Shader;
	Camera m_Camera;
	int m_Width; int m_Height;
};

#endif 
