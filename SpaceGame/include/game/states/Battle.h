#pragma once
#ifndef BATTLE_H
#define BATTLE_H

#include "GL/glew.h"
#include "core/State.hpp"
#include "core/ObjManagement.h"

#include "game/gui/GUI.h"
#include "game/states/StateRenderers.h"
#include "game/pokemon/PokemonBattle.h"

#include "game/utility/Flags.hpp"
#include "game/utility/Input.hpp"

class Battle : public State
{
public:
	void init(int width, int height, FontContainer* fonts, FlagArray* flags, GameInput* input);
	void update(double deltaTime, double time);
	void render();
	void loadRequiredData();
	void purgeRequiredData();
	void handleInput(int key, int scancode, int action, int mods);
private:
	int m_Width; int m_Height;

	//Font
	FontContainer* m_Fonts;

	//Flags
	FlagArray* m_Flags;

	//Input
	GameInput* m_Input;

	//Battle
	PokemonBattle m_Battle;

	Party m_PlayerParty;
	Party m_EnemyParty;
};

#endif