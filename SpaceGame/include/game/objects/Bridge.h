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
	BridgeRenderComponent(Struct2f pos, World::WorldHeight height, unsigned int width, unsigned int length, UVData data1, UVData data2, Render::Renderer<TextureVertex>* ren, bool horizontal);
	void generateIndices();
	void render() { if (m_Tiles.quads.size() == 0) { return; } m_Renderer->commit((TextureVertex*)&m_Tiles.quads[0], GetFloatCount<TextureVertex>(Shape::QUAD) * m_Tiles.quadCount, (unsigned int*)&m_Tiles.indices[0], m_Tiles.indices.size()); };
private:
	Render::Renderer<TextureVertex>* m_Renderer;
	QuadArray<TextureQuad> m_Tiles;
};

#endif