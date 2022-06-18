#pragma once
#ifndef BRIDGE_OBJECT_H
#define BRIDGE_OBJECT_H

#include "game/level/World.h"
#include "core/GameObject.hpp"

//Contains no new functionality, but here to allow component communication
class Bridge : public GameObject {};

//Simple limited bridge renderer - Can import model for a custom bridge
class BridgeRenderComponent : public RenderComponent
{
public:
	BridgeRenderComponent(Struct2f pos, World::WorldHeight height, unsigned int width, unsigned int length, TileUV data1, TileUV data2, Render::Renderer<NormalTextureVertex>* ren, bool horizontal);
	void generateIndices();
	void render() { if (m_Tiles.quads.size() == 0) { return; } m_Renderer->commit((NormalTextureVertex*)&m_Tiles.quads[0], GetFloatCount<NormalTextureVertex>(Shape::QUAD) * m_Tiles.quadCount, (unsigned int*)&m_Tiles.indices[0], m_Tiles.indices.size()); };
private:
	Render::Renderer<NormalTextureVertex>* m_Renderer;
	QuadArray<Norm_Tex_Quad> m_Tiles;
};

#endif