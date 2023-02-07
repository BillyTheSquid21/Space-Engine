#pragma once
#ifndef GAME_H
#define GAME_H

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "fstream"
#include "string"
#include "sstream"
#include "utility/SGUtil.h"
#include "core/GameObject.hpp"
#include "core/ObjManagement.h"
#include "renderer/RenderSys.h"
#include "Callbacks.hpp"
#include "core/Sound.h"
#include "mtlib/ThreadPool.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "core/GUI.h"

class Game 
{

public:

	//Default constructor and destructor
	Game() = default;
	~Game() = default;

	/**
	* Constructs the game instance with the given width and height
	*/
	Game(int width, int height);

	/**
	* Initialize the game
	* @param name name of the game instance
	* @param kCallback callback for key presses
	* @param mCallback callback for mouse interactions
	* @param sCallback callback for scroll interactions
	* @param windowed sets if is windowed or not - allows derived game class to specify
	*/
	bool init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback, MousePos_Callback mpCallback, bool windowed);
	
	/**
	* Poll GLFW events
	*/
	void handleEvents();
	
	/**
	* Overwrite to handle input in derived game class
	* @param key GLFW Key code
	* @param scancode GLFW Scan code
	* @param action GLFW Action code
	* @param mods GLFW modifier
	*/
	virtual void handleInput(int key, int scancode, int action, int mods) {};

	/**
	* Overwrite to handle input in derived game class
	* @param button GLFW Button code
	* @param action GLFW Action code
	* @param mods GLFW modifier
	*/
	virtual void handleMouse(int button, int action, int mods) {};
	
	/**
	* Overwrite to handle input in derived game class
	* @param xOffset Scroll x offset
	* @param yOffset Scroll y offset
	*/
	virtual void handleScrolling(double xOffset, double yOffset) {};

	/**
	* Overwrite to handle input in derived game class
	* @param xPos cursor x offset
	* @param yPos cursor y offset
	*/
	virtual void handleMousePos(double xPos, double yPos) {};

	void update(double deltaTime);
	void setTime(double time);
	void render();
	void clean();

	//Secondary Functions
	double secondsPerFrameCap() const { return m_SecondsPerFrameCap; }

	/**
	* GLFW Window Pointer
	*/
	GLFWwindow* window = NULL;

	//If this is set to true, game loop ends this cycle
	static bool s_Close;

protected:

	int m_Width;
	int m_Height;

	double m_GlfwTime;

	double m_SecondsPerFrameCap = (1.0 / 2000.0) * 1000.0; //Init to 2000, can be changed
};
#endif