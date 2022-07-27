#pragma once
#ifndef GAME_GUI_H
#define GAME_GUI_H

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "utility/SGUtil.h"
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
	void ResetStyle(); //Default style

	//Base GUI element class
	class GUIElement
	{
	public:
		GUIElement() = default;

		//Opens and closes IMGUI nest
		virtual void openNest() {  };
		virtual void closeNest() {  };
		virtual void setNest(int nest) { m_NestNumber = nest; }
		int getNest() { return m_NestNumber; }

		//Checks if contains another object within
		bool m_SelfContained = true;
		//Checks whether nest has been closed
		bool m_RenderCycleComplete = false;
		
		//Positioning
		float m_Width = 0.0f; float m_Height = 0.0f;
		float m_XPos = -1.0f; float m_YPos = -1.0f; //if negative, don't check pos (default)
		bool m_FillX = false;
		bool m_FillY = false;
		bool m_Show = true;

	protected:
		void updateDimensions(); //Adjusts dimensions - can be called to auto fill if allowed

		//Tracks which nest currently in - defaults to first
		int m_NestNumber = 0;
		float m_WindowWidth = 0; float m_WindowHeight = 0;
		float m_WindowX = 0; float m_WindowY = 0;
	};

	class GUIElementBase : public GUIElement
	{
	public:
		GUIElementBase() = default;
		GUIElementBase(float width, float height, float x, float y) { m_WindowWidth = width; m_WindowHeight = height; m_WindowX = x; m_WindowY = y; }
		virtual void setStyle() {};
		void renderStart() { GameGUI::SetNextWindowSize(m_WindowWidth, m_WindowHeight); GameGUI::SetNextWindowPos(m_WindowX, m_WindowY); }
		void renderEnd() { ImGui::End(); }
	protected:
		ImGuiWindowFlags m_Flags;
	};

	//GUI container - contains list of elements with instructions
	class GUIContainer
	{
	public:
		void setBase(std::shared_ptr<GUIElementBase> base) { m_Base = base; }
		//Elements MUST be in nest order, with each nest being located in base element
		//Different nests do not stack
		//N1->(B->E1->E2->E3,E4) N2->(B->E5->E6)
		void addElement(std::shared_ptr<GUIElement> element) { m_Elements.push_back(element); if (element->getNest() > m_NestCount) { m_NestCount = element->getNest(); } }
		void render();
		void showNest(int nest, bool show);
	private:
		//Base Element
		std::shared_ptr<GUIElementBase> m_Base = nullptr;
		//List of nested elements in order of nesting
		std::vector<std::shared_ptr<GUIElement>> m_Elements;
		int m_NestCount = 0;
	};

	//Basic Text box class
	class TextBox : public GUIElement
	{
	public:
		TextBox(std::string& str) : m_Text(str) { }
		void openNest();
	private:
		std::string& m_Text;
	};

	//Base HUD that fills whole screen and can have elements placed on
	class HUD : public GameGUI::GUIElementBase
	{
	public:
		using GUIElementBase::GUIElementBase;
		void openNest();
		void closeNest();
	};

	class Divider : public GUIElement
	{
	public:
		Divider() { m_SelfContained = false; }

		void openNest();
		void closeNest();
		void setNest(int nest) { m_NestNumber = nest; m_Name = "##Child" + std::to_string(m_NestNumber); }

	private:
		std::string m_Name = "##Child0";
	};

	class Button : public GUIElement
	{
	public:
		Button(std::string text, bool* trigger) { m_ButtonText = text; m_Trigger = trigger; };
		void openNest();
	private:
		std::string m_ButtonText;
		bool* m_Trigger = nullptr;
	};

	//Game Text box class
	class TextDisplay : public Divider
	{
	public:
		TextDisplay(std::string& text1, std::string& text2) : m_Text1Ref(text1), m_Text2Ref(text2) { };
		void setFontContainer(FontContainer* font) { m_Fonts = font; }
		void openNest();
		void closeNest();
	private:
		FontContainer* m_Fonts = nullptr;
		std::string& m_Text1Ref;
		std::string& m_Text2Ref;
	};

	class TextBuffer
	{
	public:
		void pushBuffer(std::string text);
		void setNextPageReady(bool var) { m_NextPageReady = var; } //Can externally lock whether the page is ready (eg while waiting for an animation)
		void nextPage();
		bool isReady() const { return m_IsReady; }
		bool isNextPageReady() const { return m_NextPageReady; }

		//Lines
		std::string line1;
		std::string line2;
	private:
		std::vector<char> m_Buffer;
		bool m_IsReady = true;
		bool m_NextPageReady = true;
	};

	struct TextBoxBuffer
	{
		TextBuffer buffer;
		bool showTextBox = false;
	};
}

#endif