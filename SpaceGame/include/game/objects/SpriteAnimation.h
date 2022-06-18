#pragma once
#ifndef SPRITE_ANIMATION_H
#define SPRITE_ANIMATION_H

#include <vector>

#include "core/GameObject.hpp"
#include "game/objects/TileMap.h"
#include "renderer/ShapeFactory.h"

class SpriteAnim : public UpdateComponent
{
public:
	SpriteAnim(unsigned int frameRate, unsigned int frameCount);

	void setFrame(unsigned int index, TileUV frame);
	void linkSprite(Norm_Tex_Quad* sprite, bool* animationActive) { m_Sprite = sprite; m_Active = animationActive; }
	void update(double deltaTime);

	bool loop = false;

private:
	//Timings
	float m_TimePerFrame = 0.0f;
	float m_CurrentTime = 0.0f;
	unsigned int m_CurrentFrame = 0;

	bool* m_Active = nullptr;
	Norm_Tex_Quad* m_Sprite = nullptr; //TODO make generic if need be
	std::vector<TileUV> m_Frames;
};

#endif