#pragma once
#ifndef TALL_GRASS_H
#define TALL_GRASS_H

#include "game/objects/SpriteAnimation.hpp"
#include "game/level/World.h"
#include "utility/Random.h"
#include "functional"
#include "cmath"

//Location
class TallGrassObject : public GameObject
{
public:
	//Sprites
	std::vector<glm::vec3> m_Grass;
	unsigned int m_PositionCount = 0;
	World::LevelID m_LevelID;
};

//checks if any permissions have changed and therefore whether to trigger any animations
#define GRASS_DENSITY 50.0f
class TallGrassRenderComponent : public RenderComponent
{
public:
	TallGrassRenderComponent(Render::Renderer* ren, TallGrassObject* grass) { m_Renderer = ren; m_Grass = grass; }
	
	void reserveGrass(unsigned int count) { m_Grass->m_Grass.resize(GRASS_DENSITY*GRASS_DENSITY*count); }
	void addGrass(Struct2f levelOrigin, World::Tile tile, World::WorldHeight level, World::LevelID levelID);

	void render() 
	{ 
		if (m_Grass->m_Grass.size() == 0) 
		{ 
			return; 
		} 
		m_Renderer->commitInstance(&m_Grass->m_Grass[0], 3 * m_Grass->m_Grass.size(), m_Grass->m_Grass.size());
	};
private:
	Render::Renderer* m_Renderer;
	TallGrassObject* m_Grass = nullptr;
};

#endif