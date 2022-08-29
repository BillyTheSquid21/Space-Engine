#include "game/objects/WarpTile.h"

WarpTileUpdateComponent::WarpTileUpdateComponent(OvSpr_RunningSprite* player, World::Tile loc, World::WorldHeight height, World::LevelID currentLevel, World::Tile dest, World::WorldHeight destHeight, World::LevelID levelDest)
{
	m_PlayerPointer = player; m_CurrentHeight = height; m_TileLocation = loc; m_TileDestination = dest;
	m_LevelDestination = levelDest; m_DestHeight = destHeight; m_LevelCurrent = currentLevel;
}

void WarpTileUpdateComponent::update(double deltaTime)
{
	ActionTileComponent::update(deltaTime);
	if (!m_Triggered && m_Occupied && !(m_PlayerPointer->m_Walking || m_PlayerPointer->m_Running || m_PlayerPointer->m_Busy))
	{
		//Signal fade out
		m_Fade->start();
		//Unload current level
		m_UnloadLv(m_LevelCurrent);
		//Load destination level
		m_LoadLv(m_LevelDestination);
		m_Triggered = true;
		m_PlayerPointer->m_Controlled = true;
		EngineLog("Warping to level: ", (int)m_LevelDestination);
	}
}

void WarpTileUpdateComponent::changeLevel()
{
	//Teleport to location
	Struct2f destOrigin = World::Level::queryOrigin(m_LevelDestination);
	float newXPos = destOrigin.a + m_TileDestination.x * World::TILE_SIZE + World::TILE_SIZE / 2;
	float newZPos = destOrigin.b - (m_TileDestination.z * World::TILE_SIZE) - World::TILE_SIZE / 2;
	float newYpos = ((float)m_DestHeight / sqrt(2)) * World::TILE_SIZE;
	Position<NormalTextureVertex>(&m_PlayerPointer->m_Sprite, { m_PlayerPointer->m_XPos - World::TILE_SIZE / 2, m_PlayerPointer->m_YPos, m_PlayerPointer->m_ZPos }, { newXPos - World::TILE_SIZE / 2, newYpos, newZPos }, Shape::QUAD);
	m_PlayerPointer->m_XPos = newXPos; m_PlayerPointer->m_YPos = newYpos; m_PlayerPointer->m_ZPos = newZPos;
	m_PlayerPointer->m_CurrentLevel = m_LevelDestination;
	m_PlayerPointer->m_Tile = m_TileDestination;
	m_PlayerPointer->m_WorldLevel = m_DestHeight;

	//Set new permission location
	m_PlayerPointer->m_LastPermissionPtr = World::GetTilePermission(m_PlayerPointer->m_CurrentLevel, m_PlayerPointer->m_Tile, m_PlayerPointer->m_WorldLevel);

	//Reset
	m_Occupied = false;
	m_PlayerPointer->m_Controlled = false;
}