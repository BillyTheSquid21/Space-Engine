#pragma once
#ifndef TREES_H
#define TREES_H

#include "core/GameObject.hpp"
#include "game/level/World.h"

//Contains no new functionality, but here to allow component communication
class TreeObject : public GameObject {};

//Trees are kept in one array, as their geometry shouldn't change so they get packaged together the way a plane is
class TreeRenderComponent : public RenderComponent
{
public:
	TreeRenderComponent(Render::Renderer<TextureVertex>* ren) { m_Renderer = ren; }
	void render() { if (m_Trees.size() == 0) { return; } m_Renderer->commit((TextureVertex*)&m_Trees[0], GetFloatCount<TextureVertex>(Shape::QUAD) * m_QuadCount, (unsigned int*)&m_Indices[0], m_Indices.size()); };
	//Reserve to make loading in faster
	void reserveTrees(unsigned int count) { unsigned int size = count * 2; m_Trees.resize(size); }
	void generateIndices();
	void addTree(Struct2f levelOrigin, World::TileLoc tile, World::WorldLevel level, UVData uv1, UVData uv2);
private:
	Render::Renderer<TextureVertex>* m_Renderer;
	std::vector<TextureQuad> m_Trees;
	std::vector<unsigned int> m_Indices;
	unsigned int m_QuadCount = 0;
};


#endif