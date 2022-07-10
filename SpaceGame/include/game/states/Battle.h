#pragma once
#ifndef BATTLE_H
#define BATTLE_H

#include "GL/glew.h"
#include "core/State.hpp"
#include "core/ObjManagement.h"

#include "game/gui/GUI.h"
#include "game/states/StateRenderers.h"
#include "game/pokemon/PokemonBattle.h"
#include "game/pokemon/PokemonIO.h"
#include "game/pokemon/PokemonLevel.h"
#include "game/objects/SpriteAnimation.hpp"

#include "game/utility/Flags.hpp"
#include "game/utility/Input.hpp"
#include "game/utility/ImageRead.h"

#include "mtlib/ThreadPool.h"

class BattleScene
{
public:
	void init(float width, float height);
	void render(Render::Renderer<TextureVertex>& tex, Render::Renderer<TextureVertex>& back, Render::Renderer<TextureVertex>& pok1, Render::Renderer<TextureVertex>& pok2);
	void update(double deltaTime);
private:
	Tex_Quad platform1; Tex_Quad platform2;
	Tex_Quad background;
	Tex_Quad pokemonA;
	Tex_Quad pokemonB;
	const float FRAME_COUNT = 51.0f;
	SpriteAnim<TextureVertex, Tex_Quad> pokemonAAnim;
	SpriteAnim<TextureVertex, Tex_Quad> pokemonBAnim;
	bool active = true;
};

//Base HUD that fills whole screen and can have elements placed on
class HUD : public GameGUI::GUIElementBase
{
public:
	using GUIElementBase::GUIElementBase;
	void openNest();
	void closeNest();
};

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

	//Rendering
	BattleRenderer m_Renderer;
	int m_Width; int m_Height;

	//Font
	FontContainer* m_Fonts;

	//Flags
	FlagArray* m_Flags;

	//Input
	GameInput* m_Input;

	//Battle
	PokemonBattle m_Battle;
	BattleScene m_Scene;

	//GUI
	GameGUI::GUIContainer gui;
	std::string levelA;
	std::string levelB;
	std::string nameA;
	std::string nameB;
	std::string healthA;
	std::string healthB;
	std::string conditionA;
	std::string conditionB;

	const std::string health = "Health is: ";
	const std::string status = "Status is: ";
	bool moveTriggers[4] = { false,false,false,false };
	MoveSlot selectedMove = MoveSlot::SLOT_NULL;

	Party m_PlayerParty;
	Party m_EnemyParty;

	//Thread pool for keeping battle separate
	MtLib::ThreadPool* m_Pool;
};

#endif