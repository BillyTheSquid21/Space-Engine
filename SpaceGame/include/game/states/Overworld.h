#pragma once
#ifndef OVERWORLD_H
#define OVERWORLD_H

#include "game/level/World.h"

#include "core/State.hpp"
#include "core/ObjManagement.h"

#include "game/level/WorldObjectLoader.h"
#include "game/objects/Bridge.h"
#include "game/gui/GUI.h"
#include "game/utility/GameText.h"
#include "game/states/StateRenderers.h"

#include "game/utility/Input.hpp"

#include "game/objects/TallGrass.h"


class Overworld : public State
{
public:
	void init(int width, int height, World::LevelID levelEntry, FontContainer* fonts, FlagArray* flags, GameInput* input);
	void update(double deltaTime, double time);
	void render();
	void loadRequiredData();
	void purgeRequiredData();
	void handleInput(int key, int scancode, int action, int mods);

private:
	int m_Width; int m_Height;

	//State renderer
	OverworldRenderer m_Renderer;

	//Current level - defualts to the entry level
	World::LevelID m_CurrentLevel = World::LevelID::LEVEL_ENTRY;

	//Test level
	World::LevelContainer m_Levels;
	std::shared_ptr<OvSpr_RunningSprite> sprite;

	//Test obj mng
	ObjectManager m_ObjManager;

	//Test font
	FontContainer* m_Fonts;

	//Test flags
	FlagArray* m_Flags;

	//Test GUI
	GameGUI::GUIContainer m_TextBoxGUI;
	GameGUI::GUIContainer m_DebugGUI;
	std::string m_RenderTime = "";
	std::string m_UpdateTime = "";
	std::string m_CurrentLevelStr = "";
	std::string m_CurrentTileStr = ""; 
	std::string m_ObjectCountStr = "";
	unsigned int* m_LocationX = nullptr;
	unsigned int* m_LocationZ = nullptr;
	GameGUI::TextBoxBuffer m_TextBuff;

	GameInput* m_Input;

};


#endif