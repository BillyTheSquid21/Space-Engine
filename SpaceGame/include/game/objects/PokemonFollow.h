#pragma once
#ifndef POKEMON_FOLLOW_H
#define POKEMON_FOLLOW_H

#include "core/GameObject.hpp"
#include "game/objects/OverworldSprite.h"
#include "game/objects/NPCAI.h"
#include "game/utility/PathFinding.h"
#include "game/utility/Input.hpp"

class PokemonFollow : public Ov_Sprite::TilePosition
{
public:
	using TilePosition::TilePosition;
	void linkSprites(Ov_Sprite::RunSprite* player, Ov_Sprite::RunSprite* pkm, Ov_Sprite::Sprite* ptr) { m_Player = player; m_Pkm = pkm; m_LastDestTile = m_Player->m_Tile; m_Pointer = ptr; }
	void linkScript(OverworldScript* script) { m_Script = script; }
	void linkInput(GameInput* input) { m_Input = input; }
	void update(double deltaTime);
	bool checkPerm(World::Direction direction, Struct3f pos);
private:
	OverworldScript* m_Script;
	GameInput* m_Input = nullptr;
	Ov_Sprite::RunSprite* m_Player = nullptr;
	Ov_Sprite::RunSprite* m_Pkm = nullptr;
	Ov_Sprite::Sprite* m_Pointer = nullptr;
	PathFinding::Path m_Path;
	World::Tile m_LastDestTile = { -1, -1 };
	World::Tile m_LastPkmTile;
	bool m_WasWalking = false;
	bool m_WasRunning = false;
	bool m_PtrMode = false;
	double m_Cooldown = 0.0;
};

//Class to handle pokemon animations -
class PokemonAnimation : public SGObject::UpdateComponent
{
public:
	PokemonAnimation() = default;
	PokemonAnimation(Ov_Sprite::RunSprite* spr) { m_Pkm = spr; }
	void update(double deltaTime);
private:
	Ov_Sprite::RunSprite* m_Pkm;
	unsigned int m_BaseY = 0;
	double m_Timer = 0.0;
	bool m_Frame = 0;
	World::Direction m_LastDirection = World::Direction::DIRECTION_NULL;
};

#endif