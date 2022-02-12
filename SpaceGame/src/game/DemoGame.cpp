#include "game/DemoGame.h"

bool DemoGame::init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback) {
	bool success = Game::init(name, kCallback, mCallback, sCallback);

	//Splash screen
	m_SplashScreen.init(m_Width, m_Height);

	return success;
}

void DemoGame::handleInput(int key, int scancode, int action, int mods) {

}

void DemoGame::handleMouse(int button, int action, int mods) {

}

void DemoGame::handleScrolling(double xOffset, double yOffset) {

}

void DemoGame::update(double deltaTime) {
	m_SplashScreen.update(deltaTime, m_GlfwTime);
	Game::update(deltaTime);
}

void DemoGame::render() {

	//Splash screen
	m_SplashScreen.render();

	//inherited
	Game::render();
}
