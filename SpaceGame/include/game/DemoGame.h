#pragma once
#ifndef DEMO_GAME_H
#define DEMO_GAME_H

#include "core/Game.h"
#include "game/states/Splash.h"

class DemoGame : public Game
{
public:
	using Game::Game;
	bool init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback);
	void handleInput(int key, int scancode, int action, int mods);
	void handleMouse(int button, int action, int mods);
	void handleScrolling(double xOffset, double yOffset);
	void update(double deltaTime);
	void render();

private:
	Splash m_SplashScreen;
};

#endif 
