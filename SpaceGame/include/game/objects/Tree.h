#pragma once
#ifndef TREES_H
#define TREES_H

#include "core/GameObject.hpp"
#include "game/level/World.h"

//Contains no new functionality, but here to allow component communication
class TreeObject : public SGObject::GObject {};

//Trees are kept in one array, as their geometry shouldn't change so they get packaged together the way a plane is
//Trees have 2 32X64 quads defined as there may be times someone would want to match halves of tree sprites
class TreeRenderComponent : public SGObject::RenderComponent
{
public:
	TreeRenderComponent(SGRender::Renderer* ren) { m_Renderer = ren; }
	void render() { if (m_Trees.quads.size() == 0) { return; } m_Renderer->commit((SGRender::NTVertex*)&m_Trees.quads[0], Geometry::GetFloatCount<SGRender::NTVertex>(Geometry::Shape::QUAD) * m_Trees.quadCount, (unsigned int*)&m_Trees.indices[0], m_Trees.indices.size()); };
	//Reserve to make loading in faster
	void reserveTrees(unsigned int count) { unsigned int size = count * 4; m_Trees.quads.resize(size); }
	void generateIndices();
	void addTree(Struct2f levelOrigin, World::Tile tile, World::WorldHeight level, TileUV uv1, TileUV uv2);
private:
	SGRender::Renderer* m_Renderer;
	Geometry::QuadArray<Norm_Tex_Quad> m_Trees;
};


#endif