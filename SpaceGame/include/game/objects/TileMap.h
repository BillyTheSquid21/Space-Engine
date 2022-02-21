#pragma once
#ifndef TILE_MAP_H
#define TILE_MAP_H

#include "SGUtil.h"

//|
//|Increasing Y
//|^
//||
//|
//|			->Increasing X
//|________________________

//Maps the UV coords of a texture to simple blocks of x width and x height
struct UVData
{
	float uvX;
	float uvY;
	float uvWidth;
	float uvHeight;
};

//Maps a tile to a uniform texture
class TileMap
{
public:
	TileMap(unsigned int texW, unsigned int texH, unsigned int selW, unsigned int selH);
	UVData uvTile(unsigned int tileX, unsigned int tileY);
private:
	const unsigned int m_TilesTotalX;
	const unsigned int m_TilesTotalY;
	const float m_SelectWidth;
	const float m_SelectHeight;
	static const float s_TextureSpillOffset; //Prevents borders on edge of quads especially in plane
	static const float s_PreviousTileOffset; //Prevents slightly sampling tile before in map
};

#endif