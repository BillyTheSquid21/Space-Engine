#pragma once
#ifndef GAME_GUI_H
#define GAME_GUI_H

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "SGUtil.h"
#include "renderer/RenderQueue.hpp"
#include <unordered_map>

//Create type of map which allows searches for font size
typedef std::unordered_map<unsigned char, unsigned int> FontSizeMap;

//Contains path loaded for each font style and map to allow calling unloaded sizes
struct FontSMap_FontPath
{
	FontSizeMap map;
	const char* path;
};

//Creates map of map for mapping to style, and then size
typedef std::unordered_map<std::string, FontSMap_FontPath> FontMap;

struct FontData
{
	const char* path;
	const char* name;
	unsigned char ptSize;
};

//To make clear from any ImGui stuff
namespace GameGUI 
{
	//functions
	void SetColors(int r, int g, int b, ImGuiCol target);
	void SetNextWindowSize(float width, float height);
	void SetNextWindowPos(float x, float y);
}

//Contains a map to easily access all loaded fonts
class FontContainer		//It is recommended to load any needed font for a state in the init regardless of if is loaded - in case of clearing
{
public:
	//Call once per font style loaded, all other sizes are automated
	void loadFont(const char* path, const char* fontName, unsigned char ptSize);
	ImFont* getFont(const char* fontName, unsigned char ptSize);
	void clearFonts();
private:
	FontMap m_FontMap;
};

#endif