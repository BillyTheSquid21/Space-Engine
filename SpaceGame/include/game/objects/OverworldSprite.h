#pragma once
#ifndef OVERWORLD_SPRITE_H
#define OVERWORLD_SPRITE_H

#include "core/GameObject.hpp"
#include "game/level/World.h"
#include "game/objects/TileMap.h"

//general components - can be fixed in init function or added externally per object
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
	TilePosition(World::LevelID* levelPtr, float* x, float* z, unsigned int* xTile, unsigned int* zTile) { m_CurrentLevel = levelPtr; m_XPos = x; m_ZPos = z; m_TileX = xTile; m_TileZ = zTile; };
	void update(double deltaTime);
protected:
	World::LevelID* m_CurrentLevel = nullptr;
	unsigned int* m_TileX = nullptr; unsigned int* m_TileZ = nullptr;
	float* m_XPos = nullptr; float* m_ZPos = nullptr;
};

//Maps changes to map (add origin later)
class SpriteMap : public UpdateComponent
{
public:
	SpriteMap() = default;
	SpriteMap(TextureQuad* sprite, unsigned int* animX, unsigned int* animY, TileMap* map) { m_Sprite = sprite; m_OffsetX = animX; m_OffsetY = animY; m_Map = map; };
	void update(double deltaTime);
private:
	//Offset from sprite origin
	unsigned int m_LastOffX = 0; unsigned int m_LastOffY = 0;
	unsigned int* m_OffsetX = nullptr; unsigned int* m_OffsetY = nullptr;
	TileMap* m_Map = nullptr;
	World::TileTexture m_TextureOrigin = { 0,0 };
	TextureQuad* m_Sprite;
};

class UpdateSpriteFacing : public UpdateComponent
{
public:
	UpdateSpriteFacing() = default;
	UpdateSpriteFacing(unsigned int* offset, World::Direction* direction) { m_OffsetY = offset; m_Direction = direction; }
	void update(double deltaTime);

protected:
	World::Direction* m_Direction = nullptr;
	World::Direction m_LastDirection = World::Direction::DIRECTION_NULL;
	unsigned int* m_OffsetY = nullptr;
};

class UpdateSpriteWalking : public UpdateSpriteFacing
{
public:
	UpdateSpriteWalking() = default;
	UpdateSpriteWalking(unsigned int* offsetY, unsigned int* offsetX, World::Direction* direction, bool* walking) { m_OffsetY = offsetY; m_OffsetX = offsetX; m_Direction = direction; m_Walking = walking; }
	void update(double deltaTime);

protected:
	unsigned int* m_OffsetX = nullptr;
	float m_Timer = 0.0f;
	bool* m_Walking = nullptr;
	bool m_WasWalking = false;
};   

class UpdateSpriteRunning : public UpdateSpriteWalking
{
public:
	UpdateSpriteRunning() = default;
	UpdateSpriteRunning(unsigned int* offsetY, unsigned int* offsetX, World::Direction* direction, bool* walking, bool* running) { m_OffsetY = offsetY; m_OffsetX = offsetX; m_Direction = direction; m_Walking = walking; m_Running = running; }
	void update(double deltaTime);
private:
	bool* m_Running = nullptr;
	bool m_WasRunning = false;
	bool m_ResetToZero = false;
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
	//If sprite is static, simply set here, otherwise an update method is needed
	void setSprite(UVData data);
	TextureQuad m_Sprite;
	float m_XPos = 0.0f;
	float m_YPos = 0.0f;
	float m_ZPos = 0.0f;
	unsigned int m_TileX = 0; unsigned int m_TileZ = 0;
	bool m_Walking = false;
	bool m_Running = false;
	unsigned int m_AnimationOffsetX = 0;
	unsigned int m_AnimationOffsetY = 0;
	World::Direction m_Direction = World::Direction::SOUTH;
	World::LevelID m_CurrentLevel = World::LevelID::LEVEL_NULL;
};

#endif