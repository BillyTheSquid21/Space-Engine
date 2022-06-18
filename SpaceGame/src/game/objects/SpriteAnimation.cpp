#include "game/objects/SpriteAnimation.h"

SpriteAnim::SpriteAnim(unsigned int frameRate, unsigned int frameCount)
{
	m_Frames.resize(frameCount);
	m_TimePerFrame = 1 / (float)frameRate;
}

void SpriteAnim::setFrame(unsigned int index, TileUV frame)
{
	if (index >= m_Frames.size())
	{
		EngineLog("Frame does not exist!");
		return;
	}
	m_Frames[index] = frame;
}

void SpriteAnim::update(double deltaTime)
{
	if (!(*m_Active))
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
			SetQuadUV<NormalTextureVertex>((NormalTextureVertex*)m_Sprite, uv.u, uv.v, uv.width, uv.height);
			if (!loop)
			{
				*m_Active = false;
				return;
			}
		}
		TileUV uv = m_Frames[m_CurrentFrame];
		SetQuadUV<NormalTextureVertex>((NormalTextureVertex*)m_Sprite, uv.u, uv.v, uv.width, uv.height);
	}

	m_CurrentTime += deltaTime;
}