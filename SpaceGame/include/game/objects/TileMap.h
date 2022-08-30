#pragma once
#ifndef TILE_MAP_H
#define TILE_MAP_H

#include "utility/SGUtil.h"

//|
//|Increasing Y
//|^
//||
//|
//|			->Increasing X
//|________________________

//Maps the UV coords of a texture to simple blocks of x width and x height
struct TileUV
{
	float u;
	float v;
	float width;
	float height;
};

//Maps a tile to a uniform texture
class TileMap
{
public:
	TileMap() = default;
	TileMap(unsigned int texW, unsigned int texH, unsigned int selW, unsigned int selH);
	TileUV uvTile(unsigned int tileX, unsigned int tileY);
	TileUV uvTile(unsigned int tileX, unsigned int tileY, unsigned int width, unsigned int height);
private:
	unsigned int m_TilesTotalX;
	unsigned int m_TilesTotalY;
	unsigned int m_TexWidth;
	unsigned int m_TexHeight;
	float m_SelectWidth;
	float m_SelectHeight;
	static const float s_TextureSpillOffset; //Prevents borders on edge of quads especially in plane
	static const float s_PreviousTileOffset; //Prevents slightly sampling tile before in map
};

#endif