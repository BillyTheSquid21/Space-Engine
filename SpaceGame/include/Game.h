#pragma once
#ifndef GAME_H
#define GAME_H

#include "Renderer.hpp"
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <sstream>
#include "SGUtil.h"
#include "GameObject.h"
#include "Callbacks.hpp"

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

	//render
	Shader m_ShaderProgram;
	Renderer<Vertex> m_Renderer;

	//test
	// Vertices coordinates
	float vert[35] =
	{ //     COORDINATES     /        COLORS		/
		-0.5f, 0.0f,  0.5f,     0.63f, 0.10f, 0.14f, 1.0f,
		-0.5f, 0.0f, -0.5f,     0.13f, 0.10f, 0.64f, 1.0f,
		 0.5f, 0.0f, -0.5f,     0.13f, 0.70f, 0.14f, 1.0f,
		 0.5f, 0.0f,  0.5f,     0.63f, 0.10f, 0.64f, 1.0f, 
		 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f, 1.0f
	};

	// Indices for vertices order
	unsigned int indices[18] =
	{
		0, 1, 2,
		0, 2, 3,
		0, 1, 4,
		1, 2, 4,
		2, 3, 4,
		3, 0, 4
	};
	
};
#endif