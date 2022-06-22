#pragma once
#ifndef DEMO_GAME_H
#define DEMO_GAME_H

#include "core/Game.h"
#include "game/states/Splash.h"
#include "game/states/MainMenu.h"
#include "game/states/Overworld.h"
#include "game/gui/GUI.h"

#include "mtlib/ThreadPool.h"

#define THREAD_POOL_SIZE 5

class DemoGame : public Game
{
public:
	using Game::Game;
	~DemoGame() {ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();}
	bool init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback);
	void handleInput(int key, int scancode, int action, int mods);
	void handleMouse(int button, int action, int mods);
	void handleScrolling(double xOffset, double yOffset);
	void update(double deltaTime);
	void render();

private:
	FontContainer m_Fonts;
	std::vector<std::shared_ptr<State>> m_States;
	GameInput m_GameInput;
	static FlagArray s_GlobalFlags;
};

//Defined of which entry corresponds to what state
#define SplashScreen m_States[0] 
#define MainMenuScreen m_States[1] 
#define OverworldScreen m_States[2] 

#endif 
