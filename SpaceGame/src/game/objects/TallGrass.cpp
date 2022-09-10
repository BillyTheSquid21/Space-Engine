#include "game/objects/TallGrass.h"

void TallGrassRenderComponent::addGrass(Struct2f levelOrigin, World::Tile tile, World::WorldHeight level, World::LevelID levelID)
{
	assert(m_Grass->m_PositionCount < m_Grass->m_Pos.size());

	//Dimensions
	float xPos = levelOrigin.a + tile.x * (float)World::TILE_SIZE;
	float zPos = levelOrigin.b - tile.z * (float)World::TILE_SIZE;
	float yPos = ((int)level / sqrt(2)) * World::TILE_SIZE;

	//Spread across tile
	int i = 0; //Avoids changing instance number for color or height
	for (int z = 0; z < GRASS_DENSITY; z++)
	{
		for (int x = 0; x < GRASS_DENSITY; x++)
		{
			glm::vec4 pos = { xPos + ((float)World::TILE_SIZE) * ((float)x/GRASS_DENSITY), yPos,  zPos - ((float)World::TILE_SIZE) * ((float)z / (float)GRASS_DENSITY), i };
			
			//slightly offset
			float dX = rand() % 100 - 50;
			pos.x += dX / 50.0f;

			float dZ = rand() % 100 - 50;
			pos.z += dZ / 50.0f;
			
			m_Grass->m_Pos[m_Grass->m_PositionCount] = pos;
			m_Grass->m_PositionCount++;
			i++;
		}
	}
}

void TallGrassRenderComponent::frustumCull()
{
	m_Grass->m_PositionCountCulled = 0;

	//Check for each tile whether middle is within frustum
	//This method leaves grass being rendered offscreen
	//However it massively improves performance over per blade methods
	int perTileCount = GRASS_DENSITY * GRASS_DENSITY;
	for (int i = 0; i < m_Grass->m_Pos.size(); i += perTileCount)
	{
		int corners[4] = {};
		corners[0] = i; corners[1] = i + (int)GRASS_DENSITY;
		corners[2] = i + (int)GRASS_DENSITY * (int)(GRASS_DENSITY-1);
		corners[3] = i + perTileCount -1;
		for (int j = 0; j < 4; j++)
		{
			if (checkInFrustum(m_Grass->m_Pos[corners[j]]))
			{
				std::copy(m_Grass->m_Pos.begin() + i, m_Grass->m_Pos.begin() + i + perTileCount, m_Grass->m_CulledPos.begin() + m_Grass->m_PositionCountCulled);
				m_Grass->m_PositionCountCulled += perTileCount;
				break;
			}
		}
	}
}

bool TallGrassRenderComponent::checkInFrustum(glm::vec3 pos)
{
	//Treat point as a sphere slightly larger than the tile
	SGRender::Frustum frustum = m_Camera->getFrustum();
	float threshold = -32.0f;
	return frustum.bottomFace.getSignedDistanceToPlane(pos) > threshold
		&& frustum.topFace.getSignedDistanceToPlane(pos) > threshold
		&& frustum.nearFace.getSignedDistanceToPlane(pos) > threshold
		&& frustum.farFace.getSignedDistanceToPlane(pos) > threshold
		&& frustum.leftFace.getSignedDistanceToPlane(pos) > threshold
		&& frustum.rightFace.getSignedDistanceToPlane(pos) > threshold;
}