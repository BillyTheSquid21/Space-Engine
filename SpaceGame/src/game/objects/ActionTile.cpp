#include "game/objects/ActionTile.h"

void ActionTileComponent::update(double deltaTime)
{
	if (m_PlayerPointer->m_CurrentLevel != m_LevelCurrent)
	{
		return;
	}

	if (m_TileLocation.x == m_PlayerPointer->m_Tile.x && m_TileLocation.z == m_PlayerPointer->m_Tile.z
		&& m_CurrentHeight == m_PlayerPointer->m_WorldLevel)
	{
		m_Occupied = true;
		return;
	}
	else
	{
		m_Occupied = false;
	}
}