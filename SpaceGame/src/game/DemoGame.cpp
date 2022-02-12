#include "game/DemoGame.h"

bool DemoGame::init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback) {
	bool success = Game::init(name, kCallback, mCallback, sCallback);

	//Splash screen
	m_SplashScreen.init(m_Width, m_Height);

	//IMGUi
	//Setup IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

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
