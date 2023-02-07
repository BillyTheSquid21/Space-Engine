#pragma once
#ifndef GAME_GUI
#define GAME_GUI

#include "stdint.h"
#include "unordered_map"
#include "memory"
#include "utility/SGUtil.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "core/ColorPalette.hpp"

namespace GUI
{
	class GameGUI
	{
	public:
		//Place a blank gui, all sub windows are children
		static void GUIStart(int width, int height);
		static void GUIEnd();

		//Loads fonts that are always present
		static void LoadDefaultFonts();

		//Styles to make changing look easier
		static void PushDefault();
		static void PopDefault();

		static float XOffset() { return m_XOff; }
		static float YOffset() { return m_YOff; }

	private:
		static float m_XOff;
		static float m_YOff;
	};

	//Create type of map which allows searches for font size
	typedef std::unordered_map<uint8_t, uint32_t> FontSizeMap;

	//Contains path loaded for each font style and map to allow calling unloaded sizes
	struct FontSMap_FontPath
	{
		FontSizeMap map;
		const char* path;
	};

	//Creates map of map for mapping to style, and then size
	typedef std::unordered_map<std::string, FontSMap_FontPath> FontMap;

	//Contains a map to easily access all loaded fonts
	class FontContainer		//It is recommended to load any needed font for a state in the init regardless of if is loaded - in case of clearing
	{
	public:
		static void init() { m_FontMap = std::unique_ptr<FontMap>(new FontMap()); };
		static void loadFont(const char* path, const char* fontName, unsigned char ptSize);
		static ImFont* getFont(const char* fontName, unsigned char ptSize);
		static void clear();
	private:
		static std::unique_ptr<FontMap> m_FontMap;
	};

	struct FontData
	{
		const char* path;
		const char* name;
		unsigned char ptSize;
	};
}

#endif