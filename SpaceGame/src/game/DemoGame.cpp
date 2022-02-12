#include "game/DemoGame.h"

bool DemoGame::init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback) {
	bool success = Game::init(name, kCallback, mCallback, sCallback);
	return success;
}

void DemoGame::handleInput(int key, int scancode, int action, int mods) {

}

void DemoGame::handleMouse(int button, int action, int mods) {

}

void DemoGame::handleScrolling(double xOffset, double yOffset) {

}

void DemoGame::update(double deltaTime) {

	Game::update(deltaTime);
}

void DemoGame::render() {

	//inherited
	Game::render();
}
