#include "game/objects/WarpTile.h"

WarpTileUpdateComponent::WarpTileUpdateComponent(OvSpr_RunningSprite* player, World::TileLoc loc, World::WorldHeight height, World::TileLoc dest, World::WorldHeight destHeight, World::LevelID levelDest)
{
	m_PlayerPointer = player; m_CurrentHeight = height; m_TileLocation = loc; m_TileDestination = dest;
	m_LevelDestination = levelDest; m_DestHeight = destHeight;
}

void WarpTileUpdateComponent::update(double deltaTime)
{
	if (m_PlayerPointer->m_CurrentLevel != m_LevelCurrent)
	{
		return;
	}

	if (m_TileLocation.x == m_PlayerPointer->m_Tile.x && m_TileLocation.z == m_PlayerPointer->m_Tile.z
		&& m_CurrentHeight == m_PlayerPointer->m_WorldLevel && !m_Warping)
	{
		m_Warping = true;
		return;
	}

	if (m_Warping && !(m_PlayerPointer->m_Walking || m_PlayerPointer->m_Running))
	{
		//Load destination level
		m_LoadLv(m_LevelDestination);
		//Unload current level
		m_UnloadLv(m_LevelCurrent);

		//Teleport to location
		Struct2f destOrigin = World::Level::queryOrigin(m_LevelDestination);
		float newXPos = destOrigin.a + m_TileDestination.x * World::TILE_SIZE + World::TILE_SIZE/2;
		float newZPos = destOrigin.b - (m_TileDestination.z * World::TILE_SIZE) - World::TILE_SIZE / 2;
		float newYpos = ((float)m_DestHeight / sqrt(2)) * World::TILE_SIZE;
		Position<NormalTextureVertex>(&m_PlayerPointer->m_Sprite, { m_PlayerPointer->m_XPos - World::TILE_SIZE / 2, m_PlayerPointer->m_YPos, m_PlayerPointer->m_ZPos }, { newXPos - World::TILE_SIZE/2, newYpos, newZPos }, Shape::QUAD);
		m_PlayerPointer->m_XPos = newXPos; m_PlayerPointer->m_YPos = newYpos; m_PlayerPointer->m_ZPos = newZPos;
		m_PlayerPointer->m_CurrentLevel = m_LevelDestination;
		m_PlayerPointer->m_Tile = m_TileDestination;
		m_PlayerPointer->m_WorldLevel = m_DestHeight;

		//Reset
		m_Warping = false;
	}
}