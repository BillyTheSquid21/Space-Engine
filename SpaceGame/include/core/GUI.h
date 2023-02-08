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

#define PALETTE1_EGGSHELL 0.98f, 0.953f, 0.867f
#define PALETTE1_LAUREL_GREEN 0.784f, 0.835f, 0.725f
#define PALETTE1_CAMBRIDGE_BLUE 0.561f, 0.753f, 0.663f
#define PALETTE1_GREEN_SHEEN 0.408f, 0.69f, 0.671f
#define PALETTE1_BLACK_CORAL 0.412f, 0.427f, 0.49f

#define PALETTE2_BLACK 0.0f, 0.0f, 0.0f
#define PALETTE2_POWDER_BLUE 0.722f, 0.859f, 0.851f
#define PALETTE2_GHOST_WHITE 0.957f, 0.957f, 0.976f
#define PALETTE2_ORANGE_WEB 0.957f, 0.624f, 0.039f
#define PALETTE2_BLACK_CORAL 0.412f, 0.427f, 0.49f

namespace SGGUI
{
	class GUIBase
	{
	public:
		virtual void start(float xOff, float yOff, float screenW, float screenH) {};
		virtual void end() {};
	};

	class System
	{
	public:
		static bool init(float width, float height) { s_Width = width; s_Height = height; return true; };
		static void set();
		static void clean();
		static void render();

		static int32_t addGUI(std::shared_ptr<GUIBase> gui);
		static bool removeGUI(int32_t id);
		static void setShowGUI(int32_t id, bool show);
		static bool accessGUI(int32_t id, GUIBase** gui);

		//Loads fonts that are always present
		static void loadDefaultFonts();

		static float xOffset() { return s_XOff; }
		static float yOffset() { return s_YOff; }

	private:
		//Place a blank gui, all sub windows are children
		static void start();
		static void end();

		//Styles to make changing look easier
		static void pushDefault();
		static void popDefault();

	private:

		//Stores the ptr and id of a gui
		struct GUIStorage
		{
			int32_t id;
			std::shared_ptr<GUIBase> ptr;
			bool show = false;
		};

		static std::vector<GUIStorage> s_GUIList;
		static float s_Width;
		static float s_Height;
		static float s_XOff;
		static float s_YOff;
		static bool s_Set;
		static int32_t s_NextGUIID;
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
	class FontStorage		//It is recommended to load any needed font for a state in the init regardless of if is loaded - in case of clearing
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