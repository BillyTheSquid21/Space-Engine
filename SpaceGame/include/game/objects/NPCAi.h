#pragma once
#ifndef NPC_AI_H
#define NPC_AI_H

#include <vector>
#include "game/objects/OverworldSprite.h"

class NPC_RandWalk : public TilePosition
{
public:
	NPC_RandWalk(World::Direction* direct, bool* busy, bool* walking, TextureQuad* sprite) { m_Direction = direct; m_Busy = busy; m_Walking = walking; m_Sprite = sprite;
	if (!s_Random.isSeeded()) { s_Random.seed(0.0f, MAX_SEED); } m_CoolDownTimer = s_Random.next() / 8.0f;};
	
	void setLocation(unsigned int* tileX, unsigned int* tileZ, float* x, float* z, World::LevelID* level) { m_TileX = tileX; m_TileZ = tileZ; m_XPos = x; m_ZPos = z; m_CurrentLevel = level; };

	void update(double deltaTime);
	bool* m_Busy = nullptr;

private:
	//Busy status is set by other components that carry out instructions - while busy the next instuction wont be read
	//AI class must be updated first per frame
	World::Direction* m_Direction = nullptr;

	//Applies to either walking or running, as npcs will 
	bool* m_Walking = nullptr;
	TextureQuad* m_Sprite = nullptr;
	float m_CoolDownTimer = 0.0f;
	double m_WalkTimer = 0.0;

	//Moving helpers
	bool canWalk();
	void startWalk();
	void cycleEnd();
	void randomWalk();
	void walk(double deltaTime);

	//Static randomness for randwalk class
	static RandomContainer s_Random;
	static const float MAX_SEED;
};

#endif 
