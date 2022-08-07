#pragma once
#ifndef OVERWORLD_H
#define OVERWORLD_H

#include "game/level/World.h"

#include "core/State.hpp"
#include "core/ObjManagement.h"

#include "game/level/WorldObjectLoader.h"
#include "game/objects/Bridge.h"
#include "game/gui/GUI.h"
#include "game/gui/OverworldGUI.h"
#include "game/utility/GameText.h"
#include "game/states/StateRenderers.h"

#include "game/utility/Input.hpp"

#include "game/objects/TallGrass.h"
#include "game/objects/WarpTile.h"
#include "game/objects/ScriptTile.hpp"

#include "game/data/PlayerData.hpp"
#include "game/pokemon/Pokemon.h"
#include "game/pokemon/PokemonIO.h"
#include "game/pokemon/PokemonLevel.h"

#include <functional>

class Overworld : public State
{
public:
	void init(
		int width, 
		int height, 
		PlayerData* data, 
		World::LevelID levelEntry, 
		FontContainer* fonts, 
		FlagArray* flags, 
		GameInput* input
	);

	void setBattleFunction(std::function<void(Party*, Party*)> battle);

	void update(double deltaTime, double time);
	void render();
	void loadRequiredData();
	void purgeRequiredData();
	void handleInput(int key, int scancode, int action, int mods);

private:
	int m_Width; int m_Height;

	void loadObjectData();

	//State renderer
	OverworldRenderer m_Renderer;

	//Current level - defualts to the entry level
	World::LevelID m_CurrentLevel = World::LevelID::LEVEL_ENTRY;

	//Test level
	World::LevelContainer m_Levels;
	std::shared_ptr<OvSpr_RunningSprite> m_PlayerPtr;

	//Test obj mng
	ObjectManager m_ObjManager;

	//Test font
	FontContainer* m_Fonts;

	//Test flags
	FlagArray* m_Flags;

	//Data
	PlayerData* m_Data;
	bool m_StartBattle = false;

	//Test battle
	Party enemy;

	//Test GUI
	GameGUI::GUIContainer m_HUD;
	std::string m_RenderTime = "";
	std::string m_UpdateTime = "";
	std::string m_CurrentLevelStr = "";
	std::string m_CurrentTileStr = ""; 
	std::string m_ObjectCountStr = "";
	int* m_LocationX = nullptr;
	int* m_LocationZ = nullptr;
	GameGUI::TextBoxBuffer m_TextBuff;
	float m_SampleTime = 0.0f;
	bool m_Sample = false;
	bool m_ShowDebug = false;
	bool m_ShowMenu = false;

	//Test battle
	void startBattle();
	std::function<void(Party*, Party*)> m_BattleEnable;

	GameInput* m_Input;

};


#endif