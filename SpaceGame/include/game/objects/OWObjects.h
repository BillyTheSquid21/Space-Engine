#pragma once
#ifndef OW_OBJECTS_H
#define OW_OBJECTS_H

#include "core/GameObject.hpp"
#include "game/level/World.h"
#include "game/objects/TileMap.h"

//components - can be fixed in init function or added externally per object
class SpriteRender : public RenderComponent
{
public:
	SpriteRender() = default;
	SpriteRender(TextureQuad* sprite, Renderer<TextureVertex>* ren) { m_Sprite = sprite; m_Renderer = ren; }
	void render();
private:
	TextureQuad* m_Sprite = nullptr;
	Renderer<TextureVertex>* m_Renderer = nullptr;
};

class TilePosition : public UpdateComponent
{
public:
	TilePosition() = default;
	TilePosition(World::LevelID* levelPtr, float* x, float* y, unsigned int* xTile, unsigned int* yTile) { m_CurrentLevel = levelPtr; m_XPos = x; m_YPos = y; m_TileX = xTile; m_TileY = yTile; };
	void update(double deltaTime);
protected:
	World::LevelID* m_CurrentLevel = nullptr;
	unsigned int* m_TileX = nullptr; unsigned int* m_TileY = nullptr;
	float* m_XPos = nullptr; float* m_YPos = nullptr;
};

//Player walk uses exact data of TilePosition so inherit for this
class PlayerWalk : public TilePosition
{
	using TilePosition::TilePosition;
	void update(double deltaTime);
	void setInput(bool* walking, bool* up, bool* down, bool* left, bool* right, World::Direction* direction) { m_Walking = walking; m_Direction = direction; m_Up = up; m_Down = down, m_Left = left, m_Right = right; };
private:
	bool* m_Up = nullptr; bool* m_Down = nullptr;
	bool* m_Left = nullptr; bool* m_Right = nullptr;
	double m_Timer = 0.0; bool* m_Walking = nullptr;
	World::Direction* m_Direction = nullptr;
};

//Groups for iterating contigously - when emplaced back update all pointers
class SpriteRenderGroup : public RenderComponentGroup
{
public:
	void iterate() { IterateRenderComps<SpriteRender>(m_Components); };
	void addComponent(std::vector<RenderComponent*>* compPointers, TextureQuad* sprite, Renderer<TextureVertex>* ren) { m_Components.emplace_back(sprite, ren); m_Components[m_Components.size() - 1].attachToObject(compPointers); for (int i = 0; i < m_Components.size(); i++) { m_Components[i].updatePointer(); } };
private:
	std::vector<SpriteRender> m_Components;
};

class TilePositionGroup : public UpdateComponentGroup
{
public:
	void iterate(double deltaTime) { IterateUpdateComps<TilePosition>(m_Components, deltaTime); };
	void addComponent(std::vector<UpdateComponent*>* compPointers, World::LevelID* levelPtr, float* x, float* y, unsigned int* xTile, unsigned int* yTile) { m_Components.emplace_back(levelPtr, x, y, xTile, yTile); m_Components[m_Components.size() - 1].attachToObject(compPointers); for (int i = 0; i < m_Components.size(); i++) { m_Components[i].updatePointer(); } };
private:
	std::vector<TilePosition> m_Components;
};

class OverworldSprite : public GameObject
{
public:
	OverworldSprite(float xPos, float yPos, float zPos, float width, float height);
	TextureQuad m_Sprite;
	float m_XPos = 0.0f;
	float m_YPos = 0.0f;
	float m_ZPos = 0.0f;
	unsigned int m_TileX = 0; unsigned int m_TileY = 0;
	bool m_Walking = false;
	World::Direction m_Direction = World::Direction::DIRECTION_NULL;
	World::LevelID m_CurrentLevel = World::LevelID::LEVEL_NULL;
};

#endif