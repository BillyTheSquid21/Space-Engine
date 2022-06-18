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
	TileMap(unsigned int texW, unsigned int texH, unsigned int selW, unsigned int selH);
	TileUV uvTile(unsigned int tileX, unsigned int tileY);
	TileUV uvTile(unsigned int tileX, unsigned int tileY, unsigned int width, unsigned int height);
private:
	const unsigned int m_TilesTotalX;
	const unsigned int m_TilesTotalY;
	const unsigned int m_TexWidth;
	const unsigned int m_TexHeight;
	const float m_SelectWidth;
	const float m_SelectHeight;
	static const float s_TextureSpillOffset; //Prevents borders on edge of quads especially in plane
	static const float s_PreviousTileOffset; //Prevents slightly sampling tile before in map
};

#endif