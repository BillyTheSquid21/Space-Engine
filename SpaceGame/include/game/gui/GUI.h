#pragma once
#ifndef GAME_GUI_H
#define GAME_GUI_H

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

//To make clear from any ImGui stuff
namespace GameGUI 
{
	//functions
	void SetColors(int r, int g, int b, ImGuiCol target);
	void SetNextWindowSize(float width, float height);
	void SetNextWindowPos(float x, float y);
}

#endif