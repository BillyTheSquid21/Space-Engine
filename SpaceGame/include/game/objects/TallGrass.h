#pragma once
#ifndef TALL_GRASS_H
#define TALL_GRASS_H

#include "game/objects/SpriteAnimation.hpp"
#include "game/level/World.h"
#include "game/utility/Random.hpp"

#include <functional>

//Location
struct GrassData
{
	World::Tile loc;
	World::WorldHeight height;
	World::MovementPermissions lastPermission;
};

class TallGrassObject : public GameObject
{
public:
	//Sprites
	QuadArray<Norm_Tex_Quad> m_Grass;
	std::vector<GrassData> m_GrassLoc;
	World::LevelID m_LevelID;

	//Active - bool replaced by char here
	std::vector<char> m_ActiveStates;
};

//checks if any permissions have changed and therefore whether to trigger any animations
class TallGrassAnimationComponent : public RenderComponent
{
public:
	TallGrassAnimationComponent(std::vector<GrassData>* grassLoc, World::LevelID* id, std::vector<char>* states) { m_GrassLoc = grassLoc; m_LevelID = id; m_ActiveStates = states; }
	void render();
private:
	//Sprites
	std::vector<GrassData>* m_GrassLoc = nullptr;
	World::LevelID* m_LevelID = nullptr;

	//Active - bool replaced by char here
	std::vector<char>* m_ActiveStates = nullptr;
};

class TallGrassRenderComponent : public RenderComponent
{
public:
	TallGrassRenderComponent(Render::Renderer* ren, TileUV uv, QuadArray<Norm_Tex_Quad>* grass) { m_Renderer = ren; m_UV = uv; m_Grass = grass; }
	
	void reserveGrass(unsigned int count) { m_Grass->quads.resize(count); }
	void addGrass(Struct2f levelOrigin, World::Tile tile, World::WorldHeight level, World::LevelID levelID , std::vector<GrassData>* grassLoc, std::vector<char>* states);
	void generateIndices();

	void render() { if (m_Grass->quads.size() == 0) { return; } m_Renderer->commit((NormalTextureVertex*)&m_Grass->quads[0], GetFloatCount<NormalTextureVertex>(Shape::QUAD) * m_Grass->quadCount, (unsigned int*)&m_Grass->indices[0], m_Grass->indices.size()); };

private:
	Render::Renderer* m_Renderer;
	QuadArray<Norm_Tex_Quad>* m_Grass = nullptr;
	TileUV m_UV;
};

#endif