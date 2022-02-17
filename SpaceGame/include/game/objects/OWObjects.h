#pragma once
#ifndef OW_OBJECTS_H
#define OW_OBJECTS_H

#include "core/GameObject.hpp"
#include "game/level/World.h"

//components - can be fixed in init function or added externally per object
class WorldLocation : public Component
{
public:
	WorldLocation(float& x, float& z, unsigned int& tilePointX, unsigned int& tilePointZ);
	void setLevelOrigin(Component2f tileOrigin) {m_LevelOriginX = tileOrigin.a; m_LevelOriginZ = tileOrigin.b; }
	void setTileStart(float& yPos, World::WorldLevel height, unsigned int xTile, unsigned int zTile);
	void function(double deltaTime, double time);
private:
	float m_LevelOriginX = 0.0f; float m_LevelOriginZ = 0.0f;

	//Data to read from and write to
	float& m_XRef; float& m_ZRef;
	unsigned int& m_CurrentTileX; unsigned int& m_CurrentTileZ;
};

class PlayerMove : public Component
{
public:
	PlayerMove(bool& up, bool& down, bool& left, bool& right, float& x, float& y, float& z, TextureQuad& sprite);
	void function(double deltaTime, double time);
	bool checkPermissions() { return true; };

private:
	//Walking
	World::Direction m_Direction = World::Direction::DIRECTION_NULL;
	bool m_Walking = false;
	double m_Timer = 0.0;

	//Read/write data
	float& m_XRef; float& m_YRef; float& m_ZRef;
	const bool& m_HeldUp;	const bool& m_HeldDown;
	const bool& m_HeldLeft; const bool& m_HeldRight;
	TextureQuad& m_Sprite;
};

//objects
class Sprite : public GameObject
{
public:
	void generate(float xPos, float yPos, World::LevelID level, unsigned int components);
	void render();
	void update(double deltaTime, double time);
	TextureQuad m_Sprite;
	World::LevelID m_LevelID = World::LevelID::LEVEL_NULL;
	unsigned int m_CurrentTileX; unsigned int m_CurrentTileZ;
};

class Player : public Sprite
{
public:
	void generate(float xPos, float yPos, bool& up, bool& down, bool& left, bool& right, World::LevelID level, unsigned int components);
};

#endif