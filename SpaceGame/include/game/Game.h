#pragma once
#ifndef GAME_H
#define GAME_H

#include "renderer/Renderer.hpp"
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <sstream>
#include "SGUtil.h"
#include "game/GameObject.hpp"
#include "Callbacks.hpp"
#include "renderer/Texture.h"
#include "renderer/Plane.h"

class Game 
{

public:

	//Default constructor and destructor
	Game() = default;
	~Game() = default;

	//Main constructor
	Game(int width, int height);

	//Primary functions
	bool init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback);
	void handleEvents();
	void handleInput(int key, int scancode, int action, int mods);
	void handleMouse(int button, int action, int mods);
	void handleScrolling(double xOffset, double yOffset);
	void update(double deltaTime);
	void setTime(double time);
	void render();
	void clean();

	//Secondary Functions
	double secondsPerFrameCap() const { return m_SecondsPerFrameCap; }

	//window
	GLFWwindow* window = NULL;

protected:

	int m_Width;
	int m_Height;

	double m_GlfwTime;

	double m_SecondsPerFrameCap = (1.0 / 500.0) * 1000.0; //Init to 500, can be changed

	//render - camera, program and renderer which can be moved into derived
	Camera m_Camera;
	Shader m_ShaderProgram;
	Renderer<Vertex> m_Renderer;	
};
#endif