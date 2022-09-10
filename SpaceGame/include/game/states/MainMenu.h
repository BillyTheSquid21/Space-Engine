#pragma once
#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "renderer/Renderer.hpp"
#include "renderer/Texture.h"
#include "core/State.hpp"
#include "game/gui/GUI.h"
#include "game/states/Overworld.h"
#include "core/Game.h"

class MainMenu : public SG::State
{
public:
	void init(int width, int height, GLFWwindow* window, std::shared_ptr<Overworld> overworldPtr, FontContainer* fonts);
	void update(double deltaTime, double time);
	void render();
	void loadRequiredData();
	void purgeRequiredData();
private:
	GLFWwindow* m_WindowPointer;
	SGRender::Renderer m_Renderer;
	SGRender::Shader m_Shader;
	SGRender::Camera m_Camera;
	int m_Width; int m_Height;

	FontContainer* m_Fonts;

	//Pointers to states the main menu can activate
	std::shared_ptr<Overworld> m_OverworldPtr;
};

#endif 
