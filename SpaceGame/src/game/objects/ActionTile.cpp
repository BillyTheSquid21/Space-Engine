#include "game/objects/ActionTile.h"

void ActionTileComponent::update(double deltaTime)
{
	if (m_PlayerPointer->m_CurrentLevel != m_LevelCurrent)
	{
		return;
	}

	if (m_TileLocation == m_PlayerPointer->m_Tile && m_CurrentHeight == m_PlayerPointer->m_WorldLevel)
	{
		m_Occupied = true;
		return;
	}
	else
	{
		m_Occupied = false;
	}
}