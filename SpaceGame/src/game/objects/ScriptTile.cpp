#include "game/objects/ScriptTile.h"

void ScriptTileUpdateComponent::update(double deltaTime)
{
	ActionTileComponent::update(deltaTime);
	if (m_Occupied)
	{
		m_Script.update(deltaTime);
	}
}