#pragma once
#ifndef POKEMON_FOLLOW_H
#define POKEMON_FOLLOW_H

#include "core/GameObject.hpp"
#include "game/objects/OverworldSprite.h"
#include "game/objects/NPCAI.h"
#include "game/utility/PathFinding.h"
#include "game/utility/Input.hpp"

class PokemonFollow : public TilePosition
{
public:
	using TilePosition::TilePosition;
	void linkSprites(OvSpr_RunningSprite* player, OvSpr_RunningSprite* pkm, OvSpr_Sprite* ptr) { m_Player = player; m_Pkm = pkm; m_LastDestTile = m_Player->m_Tile; m_Pointer = ptr; }
	void linkScript(OverworldScript* script) { m_Script = script; }
	void linkInput(GameInput* input) { m_Input = input; }
	void update(double deltaTime);
	bool checkPerm(World::Direction direction, Struct3f pos);
private:
	OverworldScript* m_Script;
	GameInput* m_Input = nullptr;
	OvSpr_RunningSprite* m_Player = nullptr;
	OvSpr_RunningSprite* m_Pkm = nullptr;
	OvSpr_Sprite* m_Pointer = nullptr;
	PathFinding::Path m_Path;
	World::Tile m_LastDestTile = { -1, -1 };
	World::Tile m_LastPkmTile;
	bool m_WasWalking = false;
	bool m_WasRunning = false;
	bool m_PtrMode = false;
	double m_Cooldown = 0.0;
};

//Class to handle pokemon animations -
class PokemonAnimation : public UpdateComponent
{
public:
	PokemonAnimation() = default;
	PokemonAnimation(OvSpr_RunningSprite* spr) { m_Pkm = spr; }
	void update(double deltaTime);
private:
	OvSpr_RunningSprite* m_Pkm;
	unsigned int m_BaseY = 0;
	double m_Timer = 0.0;
	bool m_Frame = 0;
	World::Direction m_LastDirection = World::Direction::DIRECTION_NULL;
};

#endif