#include "game/objects/TallGrass.h"

void TallGrassRenderComponent::addGrass(Struct2f levelOrigin, World::Tile tile, World::WorldHeight level, World::LevelID levelID)
{
	assert(m_Grass->m_PositionCount < m_Grass->m_Grass.size());

	//Dimensions
	float xPos = levelOrigin.a + tile.x * (float)World::TILE_SIZE;
	float zPos = levelOrigin.b - tile.z * (float)World::TILE_SIZE;
	float yPos = ((int)level / sqrt(2)) * World::TILE_SIZE;

	//Spread across tile
	for (int z = 0; z < GRASS_DENSITY; z++)
	{
		for (int x = 0; x < GRASS_DENSITY; x++)
		{
			glm::vec3 pos = { xPos + ((float)World::TILE_SIZE) * ((float)x/GRASS_DENSITY), yPos,  zPos - ((float)World::TILE_SIZE) * ((float)z / (float)GRASS_DENSITY) };
			
			//slightly offset
			float dX = rand() % 100 - 50;
			pos.x += dX / 50.0f;

			float dZ = rand() % 100 - 50;
			pos.z += dZ / 50.0f;
			
			m_Grass->m_Grass[m_Grass->m_PositionCount] = pos;
			m_Grass->m_PositionCount++;
		}
	}
}