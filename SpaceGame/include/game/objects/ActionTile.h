#pragma once
#ifndef ACTION_TILE_H
#define ACTION_TILE_H

#include "core/GameObject.hpp"
#include "game/objects/OverworldSprite.h"
#include "game/level/World.h"

//Base class used to sense if player is within specified tile
class ActionTileComponent : public UpdateComponent
{
public:
	ActionTileComponent() = default;
	ActionTileComponent(OvSpr_RunningSprite* player, World::TileLoc loc, World::WorldHeight height, World::LevelID id) { m_PlayerPointer = player; m_TileLocation = loc; m_CurrentHeight = height; m_LevelCurrent = id; };
	virtual void update(double deltaTime);
protected:
	World::WorldHeight m_CurrentHeight = World::WorldHeight::F0;
	World::TileLoc m_TileLocation = {0,0};
	World::LevelID m_LevelCurrent = World::LevelID::LEVEL_NULL;
	OvSpr_RunningSprite* m_PlayerPointer = nullptr;
	bool m_Occupied = false;
};

#endif
