#include "game/DemoGame.h"

//Initialise data
PlayerData DemoGame::m_Data;

bool DemoGame::init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback) {
	bool success = Game::init(name, kCallback, mCallback, sCallback);

	//IMGUi
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//Init threadpool
	MtLib::ThreadPool::Init(THREAD_POOL_SIZE);
	MtLib::ThreadPool* pool = MtLib::ThreadPool::Fetch();

	//Intialize player data
	PokemonDataBank::loadData(PkmDataType::ITEMS);
	PokemonDataBank::loadData(PkmDataType::ITEM_CATEGORIES);
	PokemonDataBank::InitializeBag(m_Data.bag);
	pool->Run(PokemonDataBank::unloadData, PkmDataType::ITEMS);
	pool->Run(PokemonDataBank::unloadData, PkmDataType::ITEM_CATEGORIES);

	//States
	std::shared_ptr<Battle> battle(new Battle());
	battle->init(m_Width, m_Height, &m_Data, &m_Fonts, &m_Data.flags, &m_GameInput);
	battle->setActive(false);
	std::shared_ptr<Overworld> overworld(new Overworld());
	overworld->init(m_Width, m_Height, &m_Data, World::LevelID::LEVEL_ENTRY, &m_Fonts, &m_Data.flags, &m_GameInput);
	overworld->setActive(false);
	std::shared_ptr<MainMenu> mainMenuScreen(new MainMenu());
	mainMenuScreen->init(m_Width, m_Height, window, overworld, &m_Fonts);
	mainMenuScreen->setActive(true);

	std::function<void(Party*, Party*)> activateBattle = std::bind(&Battle::startBattle, battle.get(), std::placeholders::_1, std::placeholders::_2);
	std::function<void(bool)> activateOverworld = std::bind(&Overworld::setActive, overworld.get(), std::placeholders::_1);
	battle->setOverworldFunction(activateOverworld); overworld->setBattleFunction(activateBattle);

	//Add states
	std::shared_ptr<State> stateMainMenuScreen = std::static_pointer_cast<State>(mainMenuScreen);
	m_States.push_back(stateMainMenuScreen);
	std::shared_ptr<State> stateOverworld = std::static_pointer_cast<State>(overworld);
	m_States.push_back(overworld);
	std::shared_ptr<State> stateBattle = std::static_pointer_cast<State>(battle);
	m_States.push_back(battle);
	
	return success;
}

void DemoGame::handleInput(int key, int scancode, int action, int mods) {
	for (int i = 0; i < m_States.size(); i++) {
		if (m_States[i]->active()) {
			m_States[i]->handleInput(key, scancode, action, mods);
		}
	}
}

void DemoGame::handleMouse(int button, int action, int mods) {

}

void DemoGame::handleScrolling(double xOffset, double yOffset) {

}

void DemoGame::update(double deltaTime) {

	bool minimumStatesActive = false;
	int states = 0;
	for (int i = 0; i < m_States.size(); i++) {
		if (m_States[i]->active()) {
			states++;
			if (!m_States[i]->hasDataLoaded()) {
				EngineLog("Loading required data at: ", i);
				m_States[i]->loadRequiredData();
			}
			minimumStatesActive = true;
			m_States[i]->update(deltaTime, m_GlfwTime);
		}
		else {
			//Unload non cached data if not active with data loaded
			if (m_States[i]->hasDataLoaded()) {
				EngineLog("Purging required data at: ", i);
				m_States[i]->purgeRequiredData();
			}
		}
	}
	//If no states are active, close program
	if (!minimumStatesActive) {
		EngineLog("No more active states: Game shutting down");
		Game::s_Close = true;
	}
	Game::update(deltaTime);

	//Update play time
	m_Data.timePlayed += deltaTime;
}

void DemoGame::render() {
	//Clear
	Render::Renderer::clearScreen();

	for (int i = 0; i < m_States.size(); i++) {
		if (m_States[i]->active() && m_States[i]->hasDataLoaded()) {
			m_States[i]->render();
		}
	}
	//Inherited
	Game::render();
}
