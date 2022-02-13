#include "game/DemoGame.h"

bool DemoGame::init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback) {
	bool success = Game::init(name, kCallback, mCallback, sCallback);

	//IMGUi
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	io.Fonts->AddFontFromFileTTF("res\\fonts\\Newsgeek\\Newsgeek.ttf", 50);

	//States
	std::shared_ptr<Splash> splashScreen(new Splash());
	splashScreen->init(m_Width, m_Height);
	splashScreen->setActive(true);
	std::shared_ptr<MainMenu> mainMenuScreen(new MainMenu());
	mainMenuScreen->init(m_Width, m_Height, window);

	//Add states
	std::shared_ptr<State> stateSplashScreen = std::static_pointer_cast<State>(splashScreen);
	m_States.push_back(stateSplashScreen);
	std::shared_ptr<State> stateMainMenuScreen = std::static_pointer_cast<State>(mainMenuScreen);
	m_States.push_back(stateMainMenuScreen);

	return success;
}

void DemoGame::handleInput(int key, int scancode, int action, int mods) {

}

void DemoGame::handleMouse(int button, int action, int mods) {

}

void DemoGame::handleScrolling(double xOffset, double yOffset) {

}

void DemoGame::update(double deltaTime) {

	for (int i = 0; i < m_States.size(); i++) {
		if (m_States[i]->active()) {
			m_States[i]->update(deltaTime, m_GlfwTime);
		}
	}
	//time to switch from splash
	if (m_GlfwTime > 0.0) {
		SplashScreen->setActive(false);
		MainMenuScreen->setActive(true);
	}

	Game::update(deltaTime);
}

void DemoGame::render() {
	//Clear
	Renderer<Vertex>::clearScreen();

	for (int i = 0; i < m_States.size(); i++) {
		if (m_States[i]->active()) {
			m_States[i]->render();
		}
	}

	//Inherited
	Game::render();
}
