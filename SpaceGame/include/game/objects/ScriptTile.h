#pragma once
#ifndef SCRIPT_TILE_H
#define SCRIPT_TILE_H

#include "game/objects/OverworldScript.hpp"
#include "game/objects/ActionTile.h"

class ScriptTile : public GameObject {};

//Only runs the script if it is in the current tile
class ScriptTileUpdateComponent : public ActionTileComponent
{
public:
	using ActionTileComponent::ActionTileComponent;
	void setScript(OverworldScript script) { m_Script = script; }
	void update(double deltaTime);
private:
	OverworldScript m_Script;
};

#endif