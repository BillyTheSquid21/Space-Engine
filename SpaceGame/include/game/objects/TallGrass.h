#pragma once
#ifndef TALL_GRASS_H
#define TALL_GRASS_H

#include "game/data/Options.h"
#include "game/objects/SpriteAnimation.hpp"
#include "game/level/World.h"
#include "utility/Random.h"
#include "functional"
#include "cmath"

//Location
class TallGrassObject : public SGObject::GObject
{
public:
	//Sprites
	std::vector<glm::vec4> m_Pos;
	std::vector<glm::vec4> m_CulledPos;
	unsigned int m_PositionCount = 0;
	unsigned int m_PositionCountCulled = 0;
	World::LevelID m_LevelID;
};

class TallGrassRenderComponent : public SGObject::RenderComponent
{
public:
	TallGrassRenderComponent(SGRender::Renderer* ren, SGRender::Camera* cam, TallGrassObject* grass) { m_Renderer = ren; m_Camera = cam; m_Grass = grass; }
	
	void reserveGrass(unsigned int count) { m_GrassDensity = Options::grassDensity; m_Grass->m_Pos.resize(m_GrassDensity * m_GrassDensity * count); m_Grass->m_CulledPos.resize(m_GrassDensity * m_GrassDensity * count); }
	void addGrass(Struct2f levelOrigin, World::Tile tile, World::WorldHeight level, World::LevelID levelID);

	void render() 
	{ 
		frustumCull();
		if (m_Grass->m_PositionCountCulled <= 0) 
		{ 
			return; 
		} 
		m_Renderer->commitInstance(&m_Grass->m_CulledPos[0], 4 * m_Grass->m_PositionCountCulled, m_Grass->m_PositionCountCulled);
	};
private:
	void frustumCull();
	bool checkInFrustum(glm::vec3 pos);
	SGRender::Renderer* m_Renderer;
	SGRender::Camera* m_Camera;
	TallGrassObject* m_Grass = nullptr;
	float m_GrassDensity = 40.0f;
};

#endif