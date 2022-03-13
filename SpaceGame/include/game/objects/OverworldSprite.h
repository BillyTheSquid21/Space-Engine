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
	SpriteMap(TextureQuad* sprite, unsigned int* animX, unsigned int* animY, TileMap* map, World::TileTexture origin) { m_Sprite = sprite; m_OffsetX = animX; m_OffsetY = animY; m_Map = map; m_TextureOrigin = origin; };
	void update(double deltaTime);
private:
	//Offset from sprite origin
	unsigned int m_LastOffX = 0; unsigned int m_LastOffY = 0;
	unsigned int* m_OffsetX = nullptr; unsigned int* m_OffsetY = nullptr;
	TileMap* m_Map = nullptr;
	World::TileTexture m_TextureOrigin = { 0,0 };
	TextureQuad* m_Sprite;
};

class UpdateAnimationFacing : public UpdateComponent
{
public:
	UpdateAnimationFacing() = default;
	UpdateAnimationFacing(unsigned int* offset, World::Direction* direction) { m_OffsetY = offset; m_Direction = direction; update(0.0); }
	void update(double deltaTime);

protected:
	World::Direction* m_Direction = nullptr;
	World::Direction m_LastDirection = World::Direction::DIRECTION_NULL;
	unsigned int* m_OffsetY = nullptr;
};

class UpdateAnimationWalking : public UpdateAnimationFacing
{
public:
	UpdateAnimationWalking() = default;
	UpdateAnimationWalking(unsigned int* offsetX, unsigned int* offsetY, World::Direction* direction, bool* walking) { m_OffsetY = offsetY; m_OffsetX = offsetX; m_Direction = direction; m_Walking = walking; }
	void update(double deltaTime);

protected:
	unsigned int* m_OffsetX = nullptr;
	float m_Timer = 0.0f;
	bool* m_Walking = nullptr;
	bool m_WasWalking = false;
};   

class UpdateAnimationRunning : public UpdateAnimationWalking
{
public:
	UpdateAnimationRunning() = default;
	UpdateAnimationRunning(unsigned int* offsetX, unsigned int* offsetY, World::Direction* direction, bool* walking, bool* running) { m_OffsetY = offsetY; m_OffsetX = offsetX; m_Direction = direction; m_Walking = walking; m_Running = running; }
	void update(double deltaTime);
private:
	bool* m_Running = nullptr;
	bool m_WasRunning = false;
	bool m_ResetToZero = false;
};

//Overworld sprite data - each adds slightly more data for functionality
struct OvSpr_SpriteData
{
	World::TileLoc tile;
	World::WorldLevel height;
	World::LevelID levelID;
	World::TileTexture texture;
};

class OvSpr_Sprite : public GameObject
{
public:
	OvSpr_Sprite(OvSpr_SpriteData data);
	//If sprite is static, simply set here, otherwise an update method is needed
	void setSprite(UVData data);
	TextureQuad m_Sprite;
	float m_XPos = 0.0f;
	float m_YPos = 0.0f;
	float m_ZPos = 0.0f;
	unsigned int m_TileX = 0; unsigned int m_TileZ = 0;
	World::LevelID m_CurrentLevel = World::LevelID::LEVEL_NULL;
	bool m_Busy = false; //allows to turn off behaviour when script running
};

class OvSpr_DirectionalSprite : public OvSpr_Sprite
{
public:
	using OvSpr_Sprite::OvSpr_Sprite;
	World::Direction m_Direction = World::Direction::SOUTH;
	unsigned int m_AnimationOffsetY = 0;
};

class OvSpr_WalkingSprite : public OvSpr_DirectionalSprite
{
public:
	using OvSpr_DirectionalSprite::OvSpr_DirectionalSprite;
	bool m_Walking = false;
	double m_Timer = 0.0;
	unsigned int m_AnimationOffsetX = 0;
};

class OvSpr_RunningSprite : public OvSpr_WalkingSprite
{
public:
	using OvSpr_WalkingSprite::OvSpr_WalkingSprite;
	bool m_Running = false;
};

namespace Ov_Translation
{
	void Walk(World::Direction* direction, float* x, float* z, TextureQuad* sprite, double deltaTime, double* walkTimer);
	template<typename T>
	void WalkSprite(T sprite, double deltaTime)
	{
		std::shared_ptr<OvSpr_WalkingSprite> spr = std::static_pointer_cast<OvSpr_WalkingSprite>(sprite);
		Walk(&spr->m_Direction, &spr->m_XPos, &spr->m_ZPos, &spr->m_Sprite, deltaTime, &spr->m_Timer);
	}
	void Run(World::Direction* direction, float* x, float* z, TextureQuad* sprite, double deltaTime, double* walkTimer);
	template<typename T>
	void RunSprite(T sprite, double deltaTime)
	{
		std::shared_ptr<OvSpr_RunningSprite> spr = std::static_pointer_cast<OvSpr_RunningSprite>(sprite);
		Run(&spr->m_Direction, &spr->m_XPos, &spr->m_ZPos, &spr->m_Sprite, deltaTime, &spr->m_Timer);
	}
	void CentreOnTile(World::LevelID currentLevel, float* x, float* z, unsigned int tileX, unsigned int tileZ, TextureQuad* sprite);
	template<typename T>
	void CentreOnTileSprite(T sprite)
	{
		std::shared_ptr<OvSpr_Sprite> spr = std::static_pointer_cast<OvSpr_WalkingSprite>(sprite);
		CentreOnTile(spr->m_CurrentLevel, &spr->m_XPos, &spr->m_ZPos, spr->m_TileX, spr->m_TileZ, &spr->m_Sprite);
	}
}

//Creation methods 
namespace Ov_ObjCreation
{
	//Allows creating sprite types, and modifies references to common component groups
	std::shared_ptr<OvSpr_Sprite> BuildSprite(OvSpr_SpriteData data, TileMap& map, RenderComponentGroup<SpriteRender>* renGrp, 
		Renderer<TextureVertex>* sprtRen);
	std::shared_ptr<OvSpr_DirectionalSprite> BuildDirectionalSprite(OvSpr_SpriteData data, TileMap& map, RenderComponentGroup<SpriteRender>* renGrp, 
		UpdateComponentGroup<SpriteMap>* sprMap, UpdateComponentGroup<UpdateAnimationFacing>* faceUp, Renderer<TextureVertex>* sprtRen);
	std::shared_ptr<OvSpr_WalkingSprite> BuildWalkingSprite(OvSpr_SpriteData data, TileMap& map, RenderComponentGroup<SpriteRender>* renGrp, 
		UpdateComponentGroup<SpriteMap>* sprMap, UpdateComponentGroup<UpdateAnimationWalking>* walkUp, Renderer<TextureVertex>* sprtRen);
	std::shared_ptr<OvSpr_RunningSprite> BuildRunningSprite(OvSpr_SpriteData data, TileMap& map, RenderComponentGroup<SpriteRender>* renGrp, 
		UpdateComponentGroup<SpriteMap>* sprMap, UpdateComponentGroup<UpdateAnimationRunning>* runUp, Renderer<TextureVertex>* sprtRen);
}

#endif