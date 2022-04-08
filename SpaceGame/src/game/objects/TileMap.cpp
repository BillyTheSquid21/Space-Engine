#include "game/objects/TileMap.h"

const float TileMap::s_TextureSpillOffset = 0.004f;
const float TileMap::s_PreviousTileOffset = 0.0005f;

TileMap::TileMap(unsigned int texW, unsigned int texH, unsigned int selW, unsigned int selH) 
	:	m_SelectWidth((float)selW/(float)texW), m_SelectHeight((float)selH/(float)texH), m_TilesTotalX(texW / selW), m_TilesTotalY(texH / selH), m_TexWidth(texW), m_TexHeight(texH) {}

UVData TileMap::uvTile(unsigned int tileX, unsigned int tileY) 
{
	UVData uvCoords = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (tileX > m_TilesTotalX || tileY > m_TilesTotalY) {
		EngineLog("Specified tile does not exist!", tileX, tileY);
		return uvCoords;
	}

	//Add less than width and height offset to starting pos to avoid spill at start
	uvCoords.u = (float)(tileX) * (m_SelectWidth + s_PreviousTileOffset);
	uvCoords.v = (float)(tileY) * (m_SelectHeight + s_PreviousTileOffset);
	uvCoords.width = m_SelectWidth - s_TextureSpillOffset;
	uvCoords.height = m_SelectHeight - s_TextureSpillOffset;

	return uvCoords;
}

UVData TileMap::uvTile(unsigned int tileX, unsigned int tileY, unsigned int width, unsigned int height)
{
	UVData uvCoords = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (tileX > m_TilesTotalX || tileY > m_TilesTotalY) {
		EngineLog("Specified tile does not exist!", tileX, tileY);
		return uvCoords;
	}

	float selW = (float)width / (float)m_TexWidth;
	float selH = (float)height / (float)m_TexHeight;

	//Add less than width and height offset to starting pos to avoid spill at start
	uvCoords.u = (float)(tileX) * (m_SelectWidth + s_PreviousTileOffset);
	uvCoords.v = (float)(tileY) * (m_SelectHeight + s_PreviousTileOffset);
	uvCoords.width = selW - s_TextureSpillOffset;
	uvCoords.height = selH - s_TextureSpillOffset;

	return uvCoords;
}