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
	void StartFrame();
	void EndFrame();
	void SetColors(int r, int g, int b, ImGuiCol target);
	void SetNextWindowSize(float width, float height);
	void SetNextWindowPos(float x, float y);

	//Base GUI class
	class GUI
	{
	public:
		GUI() = default;
		GUI(float width, float height, float x, float y) { m_WindowWidth = width; m_WindowHeight = height; m_WindowX = x; m_WindowY = y; }
	protected:
		void renderStart() {GameGUI::SetNextWindowSize(m_WindowWidth, m_WindowHeight);GameGUI::SetNextWindowPos(m_WindowX, m_WindowY);}
		void renderEnd() { ImGui::End(); }
		float m_WindowWidth; float m_WindowHeight;
		float m_WindowX; float m_WindowY;
	};

	//Text box class
	class TextBox : public GUI
	{
	public:
		TextBox(float width, float height, float x, float y) { m_WindowWidth = width; m_WindowHeight = height; m_WindowX = x; m_WindowY = y; }
		void setStyle();
		void setFontContainer(FontContainer* font) { m_Fonts = font; }
		void run(std::string& text1, std::string& text2);
	private:
		FontContainer* m_Fonts = nullptr;
	};

	struct TextBoxBuffer
	{
		std::string t1 = "";
		std::string t2 = "";
	};

	//Struct for what GUI elements to show
	struct OvGUI_Enabled
	{
		bool showTextBox = false;
	};
}

#endif