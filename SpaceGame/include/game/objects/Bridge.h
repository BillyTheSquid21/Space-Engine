#pragma once
#ifndef BRIDGE_OBJECT_H
#define BRIDGE_OBJECT_H

#include "game/level/World.h"
#include "core/GameObject.hpp"

//Contains no new functionality, but here to allow component communication
class Bridge : public SGObject::GObject {};

//Simple limited bridge renderer - Can import model for a custom bridge
class BridgeRenderComponent : public SGObject::RenderComponent
{
public:
	BridgeRenderComponent(Struct2f pos, World::WorldHeight height, unsigned int width, unsigned int length, TileUV data1, TileUV data2, SGRender::Renderer* ren, bool horizontal);
	void generateIndices();
	void render() { if (m_Tiles.quads.size() == 0) { return; } m_Renderer->commit((SGRender::NTVertex*)&m_Tiles.quads[0], Geometry::GetFloatCount<SGRender::NTVertex>(Geometry::Shape::QUAD) * m_Tiles.quadCount, (unsigned int*)&m_Tiles.indices[0], m_Tiles.indices.size()); };
private:
	SGRender::Renderer* m_Renderer;
	Geometry::QuadArray<Norm_Tex_Quad> m_Tiles;
};

#endif