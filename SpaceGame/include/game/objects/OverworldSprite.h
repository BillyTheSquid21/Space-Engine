#pragma once
#ifndef OVERWORLD_SPRITE_H
#define OVERWORLD_SPRITE_H

#include "core/GameObject.hpp"
#include "game/level/World.h"
#include "game/objects/TileMap.h"

namespace Ov_Sprite
{
	//general components - can be fixed in init function or added externally per object
	class SpriteRender : public SGObject::RenderComponent
	{
	public:
		SpriteRender() = default;
		SpriteRender(Norm_Tex_Quad* sprite, SGRender::Renderer* ren) { m_Sprite = sprite; m_Renderer = ren; }
		void render();
	private:
		Norm_Tex_Quad* m_Sprite = nullptr;
		SGRender::Renderer* m_Renderer = nullptr;
	};

	class TilePosition : public SGObject::UpdateComponent
	{
	public:
		TilePosition() = default;
		TilePosition(World::LevelID* levelPtr, float* x, float* z, World::Tile* tile) { m_CurrentLevel = levelPtr; m_XPos = x; m_ZPos = z; m_Tile = tile; };
		void update(double deltaTime);

	protected:
		World::LevelID* m_CurrentLevel = nullptr;
		World::Tile* m_Tile = nullptr;
		float* m_XPos = nullptr; float* m_ZPos = nullptr;
	};

	//Maps changes to map (add origin later)
	class SpriteMap : public SGObject::UpdateComponent
	{
	public:
		SpriteMap() = default;
		SpriteMap(Norm_Tex_Quad* sprite, unsigned int* animX, unsigned int* animY, TileMap* map, World::TileTexture origin) { m_Sprite = sprite; m_OffsetX = animX; m_OffsetY = animY; m_Map = map; m_TextureOrigin = origin; };
		void update(double deltaTime);
	private:
		//Offset from sprite origin
		unsigned int m_LastOffX = 0; unsigned int m_LastOffY = 0;
		unsigned int* m_OffsetX = nullptr; unsigned int* m_OffsetY = nullptr;
		TileMap* m_Map = nullptr;
		World::TileTexture m_TextureOrigin = { 0,0 };
		Norm_Tex_Quad* m_Sprite;
	};

	class UpdateAnimationFacing : public SGObject::UpdateComponent
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
	struct SpriteData
	{
		World::Tile tile;
		World::WorldHeight height;
		World::LevelID levelID;
		World::TileTexture texture;
	};

	class Sprite : public SGObject::GObject
	{
	public:
		Sprite(SpriteData data, bool block);
		//If sprite is static, simply set here, otherwise an update method is needed
		void setSprite(TileUV data);
		float m_XPos = 0.0f;
		float m_YPos = 0.0f;
		float m_ZPos = 0.0f;
		World::WorldHeight m_WorldLevel = World::WorldHeight::F0;
		Norm_Tex_Quad m_Sprite;
		World::Tile m_Tile = { 0,0 };
		World::LevelID m_CurrentLevel = World::LevelID::LEVEL_NULL;
		bool m_Busy = false; //allows to turn off behaviour when script running
	};

	class DirectionSprite : public Sprite
	{
	public:
		using Sprite::Sprite;
		World::Direction m_Direction = World::Direction::SOUTH;
		unsigned int m_AnimationOffsetY = 0;
		unsigned int m_AnimationOffsetX = 0;
	};

	class WalkSprite : public DirectionSprite
	{
	public:
		using DirectionSprite::DirectionSprite;

		//Ensure pointers are nulled
		void messageAll(uint32_t message) {
			messageAllRender(message); messageAllUpdate(message);
			if (message == (uint32_t)SGObject::Message::KILL) { m_Dead = true; m_LastPermissionPtr = nullptr; }
		};

		bool m_Walking = false;
		double m_Timer = 0.0;

		//For movable sprites, store the type of last permission and location
		//Ensures is returned to currect type at correct index
		World::MovementPermissions m_LastPermission = World::MovementPermissions::CLEAR;
		World::MovementPermissions* m_LastPermissionPtr = nullptr;

		//Ascend stores whether was just going up or down
		//OnSlope stores whether is on a slope to get order of moving along and up slope right
		char m_Ascend = 0; //0 if no, 1 if up, -1 if down
		bool m_CurrentIsSlope = false; //Says whether currently on slope
		bool m_NextIsSlope = false;
		bool m_MoveVerticalFirst = false;
	};

	class RunSprite : public WalkSprite
	{
	public:
		using WalkSprite::WalkSprite;
		bool m_Running = false;
		bool m_Controlled = false; //Intended for player use
	};
}

namespace Ov_Translation
{
	//TODO - remove old walksprite type methods
	void Walk(World::Direction* direction, float* x, float* z, Norm_Tex_Quad* sprite, double deltaTime, double* walkTimer);
	void Run(World::Direction* direction, float* x, float* z, Norm_Tex_Quad* sprite, double deltaTime, double* walkTimer);
	void CentreOnTile(World::LevelID currentLevel, World::WorldHeight worldLevel, float* x, float* y, float* z, World::Tile tile, Norm_Tex_Quad* sprite);
	void CentreOnTile(World::LevelID currentLevel, World::WorldHeight worldLevel, float* x, float* y, float* z, World::Tile tile, Norm_Tex_Quad* sprite, bool onSlope);

	//Slope ascending and descending - balanced for smoothness
	void AscendSlope(float* y, Norm_Tex_Quad* sprite, double deltaTime, bool running);
	void AscendSlope(float* y, Norm_Tex_Quad* sprite, double deltaTime, bool running, double timer, bool verticalFirst);	//Allows ascending for middle of tile (won't want to ascend immediately)
	void DescendSlope(float* y, Norm_Tex_Quad* sprite, double deltaTime, bool running);
	void DescendSlope(float* y, Norm_Tex_Quad* sprite, double deltaTime, bool running, double timer, bool verticalFirst);	//Allows descending for middle of tile (won't want to ascend immediately)

	static void CheckAscend(Ov_Sprite::WalkSprite* spr);
	static void WalkMethod(Ov_Sprite::WalkSprite* spr, double deltaTime);
	static void RunMethod(Ov_Sprite::RunSprite* spr, double deltaTime);
	static void CycleEnd(Ov_Sprite::WalkSprite* spr);
	static void CycleEnd(Ov_Sprite::RunSprite* spr);
	static void CentreSprite(Ov_Sprite::WalkSprite* spr);

	bool SpriteWalk(Ov_Sprite::WalkSprite* spr, double deltaTime);
	bool SpriteRun(Ov_Sprite::RunSprite* spr, double deltaTime);

	bool CheckCanWalkNPC(Ov_Sprite::WalkSprite* spr);
	bool CheckCanWalk(Ov_Sprite::WalkSprite* spr);
}

//Creation methods 
namespace Ov_ObjCreation
{
	//Allows creating sprite types, and modifies references to common component groups
	std::shared_ptr<Ov_Sprite::Sprite> BuildSprite(Ov_Sprite::SpriteData data, TileMap& map, SGObject::RenderCompGroup<Ov_Sprite::SpriteRender>* renGrp,
		SGRender::Renderer* sprtRen, bool block);
	std::shared_ptr<Ov_Sprite::DirectionSprite> BuildDirectionalSprite(Ov_Sprite::SpriteData data, TileMap& map, SGObject::RenderCompGroup<Ov_Sprite::SpriteRender>* renGrp,
		SGObject::UpdateCompGroup<Ov_Sprite::SpriteMap>* sprMap, SGObject::UpdateCompGroup<Ov_Sprite::UpdateAnimationFacing>* faceUp, SGRender::Renderer* sprtRen);
	std::shared_ptr<Ov_Sprite::WalkSprite> BuildWalkingSprite(Ov_Sprite::SpriteData data, TileMap& map, SGObject::RenderCompGroup<Ov_Sprite::SpriteRender>* renGrp,
		SGObject::UpdateCompGroup<Ov_Sprite::SpriteMap>* sprMap, SGObject::UpdateCompGroup<Ov_Sprite::UpdateAnimationWalking>* walkUp, SGRender::Renderer* sprtRen);
	std::shared_ptr<Ov_Sprite::RunSprite> BuildRunningSprite(Ov_Sprite::SpriteData data, TileMap& map, SGObject::RenderCompGroup<Ov_Sprite::SpriteRender>* renGrp,
		SGObject::UpdateCompGroup<Ov_Sprite::SpriteMap>* sprMap, SGObject::UpdateCompGroup<Ov_Sprite::UpdateAnimationRunning>* runUp, SGRender::Renderer* sprtRen);
	std::shared_ptr<Ov_Sprite::RunSprite> BuildRunningSprite(Ov_Sprite::SpriteData data, TileMap& map, SGObject::RenderCompGroup<Ov_Sprite::SpriteRender>* renGrp,
		SGObject::UpdateCompGroup<Ov_Sprite::SpriteMap>* sprMap, SGRender::Renderer* sprtRen);
}

#endif