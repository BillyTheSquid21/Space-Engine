#pragma once
#ifndef EXAMPLE_SCENE
#define EXAMPLE_SCENE

#include "core/Game.h"

class ExampleScene : public Game
{
public:
	using Game::Game;

	bool init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback, MousePos_Callback mpCallback, bool windowed);

	void clean();

	virtual void handleInput(int key, int scancode, int action, int mods);

private:

};

#endif