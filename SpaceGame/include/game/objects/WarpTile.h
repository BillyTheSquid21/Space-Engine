#pragma once
#ifndef WARP_TILE_H
#define WARP_TILE_H

#include "functional"
#include "game/objects/ActionTile.h"

class WarpTile : public GameObject {};

class WarpTileUpdateComponent : public ActionTileComponent
{
public:
	WarpTileUpdateComponent(OvSpr_RunningSprite* player, World::Tile loc, World::WorldHeight height, World::LevelID currentLevel, World::Tile dest, World::WorldHeight destHeight, World::LevelID levelDest);
	void setLoadingFuncs(std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld) { m_LoadLv = ld; m_UnloadLv = uld; };
	void update(double deltaTime);

	//Overwrite message process so will only change level when being unloaded
	void processMessages() {
		while (m_MessageQueue.size() > 0) {
			uint32_t message = m_MessageQueue.front();
			if (message == (uint32_t)Message::ACTIVATE) { setActive(true); }
			else if (message == (uint32_t)Message::DEACTIVATE) { setActive(false); }
			else if (message == (uint32_t)Message::KILL) 
			{
				//Modify processing so once level is loaded then teleports
				if (m_Occupied)
				{
					changeLevel();
				}
				setActive(false);
			}
			m_MessageQueue.pop();
		};
	}

private:
	World::WorldHeight m_DestHeight;
	World::Tile m_TileDestination;
	World::LevelID m_LevelDestination;

	void changeLevel();

	//Loading funcs
	std::function<void(World::LevelID)> m_LoadLv;
	std::function<void(World::LevelID)> m_UnloadLv;
};

#endif