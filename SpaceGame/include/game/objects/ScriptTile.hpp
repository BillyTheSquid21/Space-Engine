#pragma once
#ifndef SCRIPT_TILE_H
#define SCRIPT_TILE_H

#include "game/objects/OverworldScript.h"
#include "game/objects/ActionTile.h"

class ScriptTile : public GameObject {};

//Only runs the script if it is in the current tile
template<typename ScriptType>
class ScriptTileUpdateComponent : public ActionTileComponent
{
public:
	ScriptTileUpdateComponent(OvSpr_RunningSprite* player, World::Tile loc, World::WorldHeight height, World::LevelID id) { m_PlayerPointer = player; m_TileLocation = loc; m_CurrentHeight = height; m_LevelCurrent = id; };
	void setScript(ScriptType script) { static_assert(std::is_base_of<OverworldScript, ScriptType>::value); m_Script = script; }
	void update(double deltaTime)
	{
		ActionTileComponent::update(deltaTime);
		if (m_Occupied)
		{
			m_Script.update(deltaTime);
		}
	}
private:
	ScriptType m_Script;
};

#endif