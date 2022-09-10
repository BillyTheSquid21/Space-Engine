#pragma once
#ifndef ACTION_TILE_H
#define ACTION_TILE_H

#include "core/GameObject.hpp"
#include "game/objects/OverworldSprite.h"
#include "game/level/World.h"

//Base class used to sense if player is within specified tile
class ActionTileComponent : public SGObject::UpdateComponent
{
public:
	ActionTileComponent() = default;
	ActionTileComponent(Ov_Sprite::RunSprite* player, World::Tile loc, World::WorldHeight height, World::LevelID id) { m_PlayerPointer = player; m_TileLocation = loc; m_CurrentHeight = height; m_LevelCurrent = id; };
	virtual void update(double deltaTime);
protected:
	World::WorldHeight m_CurrentHeight = World::WorldHeight::F0;
	World::Tile m_TileLocation = {0,0};
	World::LevelID m_LevelCurrent = World::LevelID::LEVEL_NULL;
	Ov_Sprite::RunSprite* m_PlayerPointer = nullptr;
	bool m_Occupied = false;
};

#endif
