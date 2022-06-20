#pragma once
#ifndef WARP_TILE_H
#define WARP_TILE_H

#include <functional>

#include "core/GameObject.hpp"
#include "game/objects/OverworldSprite.h"
#include "game/level/World.h"

class WarpTile : public GameObject {};

class WarpTileUpdateComponent : public UpdateComponent
{
public:
	WarpTileUpdateComponent(OvSpr_RunningSprite* player, World::TileLoc loc, World::WorldHeight height, World::TileLoc dest, World::WorldHeight destHeight, World::LevelID levelDest);
	void setLoadingFuncs(std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld) { m_LoadLv = ld; m_UnloadLv = uld; };
	void update(double deltaTime);
private:
	World::WorldHeight m_CurrentHeight;
	World::WorldHeight m_DestHeight;
	World::TileLoc m_TileLocation;
	World::TileLoc m_TileDestination;
	World::LevelID m_LevelCurrent;
	World::LevelID m_LevelDestination;
	OvSpr_RunningSprite* m_PlayerPointer = nullptr;
	bool m_Warping = false;

	//Loading funcs
	std::function<void(World::LevelID)> m_LoadLv;
	std::function<void(World::LevelID)> m_UnloadLv;
};

#endif