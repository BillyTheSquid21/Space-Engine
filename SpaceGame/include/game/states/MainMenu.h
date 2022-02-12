#pragma once
#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "renderer/Renderer.hpp"
#include "renderer/Texture.h"
#include "core/State.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

class MainMenu : public State
{
public:
	void init(int width, int height);
	void update(double deltaTime, double time);
	void render();
private:
	Renderer<Vertex> m_Renderer;
	Shader m_Shader;
	Camera m_Camera;
	int m_Width; int m_Height;
};

#endif 
