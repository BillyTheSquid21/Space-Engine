#pragma once
#ifndef OW_OBJECTS_H
#define OW_OBJECTS_H

#include "core/GameObject.hpp"
#include "game/level/World.h"
#include "game/objects/TileMap.h"

//components - can be fixed in init function or added externally per object
class SpriteRender : public RenderComponent
{
public:
	SpriteRender(TextureQuad& sprite, Renderer<TextureVertex>* ren) : m_Sprite(sprite) { m_Renderer = ren; }
	void render();
private:
	Renderer<TextureVertex>* m_Renderer;
	TextureQuad& m_Sprite;
};

class SpriteRenderGroup : public RenderComponentGroup
{
public:
	void iterate() { for (int i = 0; i < m_Components.size(); i++) { m_Components[i].render(); } }
	void addComponent(std::vector<RenderComponent*>* compPointers, TextureQuad& sprite, Renderer<TextureVertex>* ren) { m_Components.push_back(SpriteRender(sprite, ren)); m_Components[0].attachToObject(compPointers); m_Components[0].updatePointer(); };
private:
	std::vector<SpriteRender> m_Components;
};

class Sprite : public GameObject
{
public:
	Sprite() { m_Sprite = CreateTextureQuad(0.0f, 0.0f, 32.0f, 32.0f, 0.0f, 0.0f, 0.05f, 0.1f); };
	TextureQuad m_Sprite;
	float m_XPos = 0.0f;
	float m_YPos = 0.0f;
};

#endif