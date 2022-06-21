#pragma once
#ifndef WARP_TILE_H
#define WARP_TILE_H

#include <functional>

#include "game/objects/ActionTile.h"

class WarpTile : public GameObject {};

class WarpTileUpdateComponent : public ActionTileComponent
{
public:
	WarpTileUpdateComponent(OvSpr_RunningSprite* player, World::TileLoc loc, World::WorldHeight height, World::LevelID currentLevel, World::TileLoc dest, World::WorldHeight destHeight, World::LevelID levelDest);
	void setLoadingFuncs(std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld) { m_LoadLv = ld; m_UnloadLv = uld; };
	void update(double deltaTime);
private:
	World::WorldHeight m_DestHeight;
	World::TileLoc m_TileDestination;
	World::LevelID m_LevelDestination;

	//Loading funcs
	std::function<void(World::LevelID)> m_LoadLv;
	std::function<void(World::LevelID)> m_UnloadLv;
};

#endif