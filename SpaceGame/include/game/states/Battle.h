#pragma once
#ifndef BATTLE_H
#define BATTLE_H

#include "GL/glew.h"
#include "core/State.hpp"
#include "core/ObjManagement.h"

#include "game/gui/GUI.h"
#include "game/gui/BattleGUI.h"
#include "game/states/StateRenderers.h"
#include "game/pokemon/PokemonBattle.h"
#include "game/pokemon/PokemonIO.h"
#include "game/pokemon/PokemonLevel.h"
#include "game/objects/SpriteAnimation.hpp"

#include "game/data/PlayerData.hpp"

#include "game/utility/Flags.hpp"
#include "game/utility/Input.hpp"
#include "game/utility/ImageRead.h"

#include "mtlib/ThreadPool.h"

class BattleScene
{
public:
	void init(float width, float height);
	void render(StateRender::Battle& ren);
	void setPokemonA(std::string name);
	void buildPlatformA();
	void buildPlatformB();
	void setPokemonB(std::string name);
	void update(double deltaTime);
private:
	int m_Width = 0; int m_Height = 0;
	float m_SpriteWidth = 0.0f;

	//Platforms
	struct PlatformData
	{
		float depth = 1.0f;
		float elevation = 1.0f;
		float offset = 1.0f;
	};
	Tex_Quad platform1; Tex_Quad platform2;
	PlatformData platform1Data; PlatformData platform2Data;

	Tex_Quad background;
	Tex_Quad pokemonA;
	Tex_Quad pokemonB;
	SpriteAnim<SGRender::TVertex, Tex_Quad> pokemonAAnim;
	SpriteAnim<SGRender::TVertex, Tex_Quad> pokemonBAnim;
	bool active = true;
};

class Battle : public SG::State
{
public:
	void init(
		int width, 
		int height, 
		PlayerData* data, 
		FontContainer* fonts, 
		FlagArray* flags, 
		GameInput* input
	);
	
	void setOverworldFunction(std::function<void(bool)> overworld);
	void update(double deltaTime, double time);
	void render();
	void loadRequiredData();
	void purgeRequiredData();
	void handleInput(int key, int scancode, int action, int mods);
	void startBattle(Party* partyA, Party* partyB);
	void startBattleOverworld(Party* partyA, Party* partyB);

	//Set pokemon sprite in a given slot
	void setPokemonA(uint16_t id);
	void setPokemonB(uint16_t id);
private:

	//Rendering
	StateRender::Battle m_Renderer;
	int m_Width; int m_Height;

	//Font
	FontContainer* m_Fonts;

	//Flags
	FlagArray* m_Flags;

	//Input
	GameInput* m_Input;

	//Data
	PlayerData* m_Data;

	//Battle
	std::shared_ptr<PokemonBattle> m_Battle;
	BattleScene m_Scene;
	GameGUI::GUIContainer m_HUD;
	std::shared_ptr<BattleGUI> m_GUI;

	bool m_MoveTriggers[4] = { false,false,false,false };
	bool m_Exit = false;
	bool m_ChangePkm = false;
	MoveSlot selectedMove = MoveSlot::SLOT_NULL;

	Party* m_PlayerParty = nullptr;
	Party* m_EnemyParty = nullptr;

	void endBattle();

	std::function<void(bool)> m_OverworldEnable;
	bool m_DeallocateEnemy = false; //For if enemy is not stored

	//Thread pool for keeping battle separate
	MtLib::ThreadPool* m_Pool;
};

#endif