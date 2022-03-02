#include "game/objects/OverworldSprite.h"

//DEBUG
static void logDir(World::Direction dir)
{
	switch (dir)
	{
	case World::Direction::NORTH:
		EngineLog("NORTH");
		break;
	case World::Direction::SOUTH:
		EngineLog("SOUTH");
		break;
	case World::Direction::EAST:
		EngineLog("EAST");
		break;
	case World::Direction::WEST:
		EngineLog("WEST");
		break;
	default:
		EngineLog("WTF??");
		break;
	}
}

//components
void SpriteRender::render()
{
	if (m_Sprite != nullptr && m_Renderer != nullptr) {
		m_Renderer->commit((TextureVertex*)m_Sprite, GetFloatCount<TextureVertex>(Shape::QUAD), Primitive::Q_IND, Primitive::Q_IND_COUNT);
	}
}

void TilePosition::update(double deltaTime)
{
	//Works out current tile
	Component2f origin = World::Level::queryOrigin(*m_CurrentLevel);
	float deltaX = *m_XPos - origin.a;
	float deltaZ = *m_ZPos - origin.b;
	*m_TileX = (unsigned int)(deltaX / World::TILE_SIZE);
	*m_TileZ = (unsigned int)(-deltaZ / World::TILE_SIZE);
}

void SpriteMap::update(double deltaTime)
{
	if (m_LastOffX != *m_OffsetX || m_LastOffY != *m_OffsetY)
	{
		//Update sprite map
		UVData data = m_Map->uvTile(m_TextureOrigin.textureX + *m_OffsetX, m_TextureOrigin.textureY + *m_OffsetY);
		SetQuadUV((TextureVertex*)m_Sprite, data.uvX, data.uvY, data.uvWidth, data.uvHeight);
		m_LastOffX = *m_OffsetX; m_LastOffY = *m_OffsetY;
	}
}

void UpdateSpriteFacing::update(double deltaTime)
{
	if (*m_Direction != m_LastDirection)
	{
		//(East is default)
		switch (*m_Direction)
		{
		case World::Direction::WEST:
			*m_OffsetY = 1;
			break;
		case World::Direction::SOUTH:
			*m_OffsetY = 2;
			break;
		case World::Direction::NORTH:
			*m_OffsetY = 3;
			break;
		default:
			*m_OffsetY = 0;
			break;
		}

		m_LastDirection = *m_Direction;
	}
}

void UpdateSpriteWalking::update(double deltaTime)
{
	UpdateSpriteFacing::update(deltaTime);

	if (*m_Walking)
	{
		if (m_Timer <= 0.25f)
		{
			*m_OffsetX = 1;
		}
		else if (m_Timer > 0.25f && m_Timer <= 0.5f)
		{
			*m_OffsetX = 0;
		}
		else if (m_Timer > 0.5f && m_Timer <= 0.75f)
		{
			*m_OffsetX = 2;
		}
		else if (m_Timer > 0.75f && m_Timer <= 1.0f)
		{
			*m_OffsetX = 0;
		}
		else
		{
			*m_OffsetX = 1;
			m_Timer = 0.0f;
		}
		m_Timer += deltaTime;
		m_WasWalking = true;
	}
	else if (m_WasWalking)
	{
		*m_OffsetX = 0;
		m_Timer = 0.0f;
		m_WasWalking = false;
	}
}

void UpdateSpriteRunning::update(double deltaTime)
{
	UpdateSpriteWalking::update(deltaTime);
	if (*m_Running)
	{
		if (m_Timer <= 0.125f)
		{
			*m_OffsetX = 4;
		}
		else if (m_Timer > 0.125f && m_Timer <= 0.25f)
		{
			*m_OffsetX = 3;
		}
		else if (m_Timer > 0.25f && m_Timer <= 0.375f)
		{
			*m_OffsetX = 5;
		}
		else if (m_Timer > 0.375f && m_Timer <= 0.5f)
		{
			*m_OffsetX = 3;
		}
		else
		{
			*m_OffsetX = 4;
			m_Timer = 0.0f;
		}
		m_Timer += deltaTime;
		m_WasRunning = true;
		return;
	}

	if (m_WasRunning)
	{
		*m_OffsetX = 3;
		m_Timer = 0.0f;
		m_WasRunning = false;
		m_ResetToZero = true;
		return;
	}

	//Final check to avoid modifying every frame
	if (m_ResetToZero)
	{
		*m_OffsetX = 0;
		m_ResetToZero = false;
	}
}

//Objects
OverworldSprite::OverworldSprite(float xPos, float yPos, float zPos, float width, float height)
{
	//Set pos - x in tile middle
	m_XPos = xPos + World::TILE_SIZE / 2; m_YPos = yPos; m_ZPos = zPos - World::TILE_SIZE / 2;
	//Make Sprite
	m_Sprite = CreateTextureQuad(xPos, yPos + height, width, height, 0.0f, 0.0f, 0.05f, 0.1f);
	//Position Sprite
	RotateShape<TextureVertex>((TextureVertex*)&m_Sprite, { xPos + width / 2, yPos, 0.0f }, -45.0f, Shape::QUAD, Axis::X);
	TranslateShape<TextureVertex>((TextureVertex*)&m_Sprite, 0.0f, 0.0f, m_ZPos, Shape::QUAD);
}

void OverworldSprite::setSprite(UVData data)
{
	SetQuadUV((TextureVertex*)&m_Sprite, data.uvX, data.uvY, data.uvWidth, data.uvHeight);
}