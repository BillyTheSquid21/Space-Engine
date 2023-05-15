#pragma once
#ifndef EXAMPLE_SCENE
#define EXAMPLE_SCENE

#include "core/Game.h"

class ExampleScene : public Game
{
public:
	using Game::Game;

	bool init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback, MousePos_Callback mpCallback, bool windowed);

	void update(double deltaTime);

	void render();

	void clean();

	void handleInput(int key, int scancode, int action, int mods);

private:

	void initRenderer1();
	void renderRenderer1();
	void cleanRenderer1();

	SGRender::ModelID model;
	SGRender::ShaderID shader;
	std::vector<SGRender::RenderInstruction> instr;
	glm::mat4 id = glm::identity<glm::mat4>();

	SGRender::System renderer1;
	float m_ElapsedTime = 0.0f;
	int m_ShowLights = 0;
};

#endif