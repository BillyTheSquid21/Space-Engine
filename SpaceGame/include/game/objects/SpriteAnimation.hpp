#pragma once
#ifndef SPRITE_ANIMATION_H
#define SPRITE_ANIMATION_H

#include <vector>

#include "core/GameObject.hpp"
#include "game/objects/TileMap.h"
#include "renderer/Geometry.h"

template<typename Vert, typename Q>
class SpriteAnim : public SGObject::UpdateComponent
{
public:
	SpriteAnim() = default;
	SpriteAnim(unsigned int frameRate, unsigned int frameCount)
	{
		m_Frames.resize(frameCount);
		m_TimePerFrame = 1 / (float)frameRate;
	}

	void setFrame(unsigned int index, TileUV frame)
	{
		if (index >= m_Frames.size())
		{
			EngineLog("Frame does not exist!");
			return;
		}
		m_Frames[index] = frame;
	}
	void linkSprite(Q* sprite, bool* animationActive) { m_Sprite = sprite; m_Active = animationActive; }
	void update(double deltaTime)
	{
		if (!m_Active || !(*m_Active))
		{
			return;
		}
		if (m_CurrentTime >= m_TimePerFrame)
		{
			m_CurrentFrame++;
			m_CurrentTime = 0.0f;

			if (m_CurrentFrame >= m_Frames.size())
			{
				m_CurrentFrame = 0;
				TileUV uv = m_Frames[m_CurrentFrame];
				Geometry::SetQuadUV<Vert>((Vert*)m_Sprite, uv.u, uv.v, uv.width, uv.height);
				if (!loop)
				{
					*m_Active = false;
					return;
				}
			}
			TileUV uv = m_Frames[m_CurrentFrame];
			Geometry::SetQuadUV<Vert>((Vert*)m_Sprite, uv.u, uv.v, uv.width, uv.height);
		}

		m_CurrentTime += deltaTime;
	}

	bool loop = false;

private:
	//Timings
	float m_TimePerFrame = 0.0f;
	float m_CurrentTime = 0.0f;
	unsigned int m_CurrentFrame = 0;

	bool* m_Active = nullptr;
	Q* m_Sprite = nullptr; //TODO make generic if need be
	std::vector<TileUV> m_Frames;
};

#endif