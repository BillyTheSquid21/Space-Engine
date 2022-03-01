#include "game/objects/OWObjects.h"

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

//render
SpriteMap::SpriteMap(World::TileTexture texOrigin, World::Direction* direction, TileMap* tileMap, TextureQuad* sprite, unsigned int* animOff, bool* walk)
{ 
	m_TextureOrigin = texOrigin; 
	m_TileMap = tileMap; 
	m_Direction = direction; 
	m_Sprite = sprite; 
	m_LastDirection = *direction; 
	m_AnimationOffset = animOff;
	m_Walking = walk;

	//Set sprite
	World::TileTexture textureLoc = directionRow(*m_Direction, m_TextureOrigin);
	UVData uvDat = m_TileMap->uvTile(textureLoc.textureX, textureLoc.textureY);
	SetQuadUV((TextureVertex*)m_Sprite, uvDat.uvX, uvDat.uvY, uvDat.uvWidth, uvDat.uvHeight);
}
//TODO - make more effective at managing
void SpriteMap::render()
{
	//If isn't last direction, update sprite
	//if (*m_Direction != m_LastDirection || *m_Walking) 
	//{
		World::TileTexture textureLoc = directionRow(*m_Direction, m_TextureOrigin);
		UVData uvDat = m_TileMap->uvTile(textureLoc.textureX + *m_AnimationOffset, textureLoc.textureY);
		SetQuadUV((TextureVertex*)m_Sprite, uvDat.uvX, uvDat.uvY, uvDat.uvWidth, uvDat.uvHeight);
		m_LastDirection = *m_Direction;
	//}
}

void SpriteWalkMap::update(double deltaTime)
{
	if (*m_IsWalking)
	{
		m_Timer += deltaTime;
		if (m_Timer > 0.25f && m_Timer <= 0.5f)
		{
			*m_AnimationOffset = 1;
		}
		else if (m_Timer > 0.5f && m_Timer <= 0.75f)
		{
			*m_AnimationOffset = 0;
		}
		else if (m_Timer > 0.75f && m_Timer < 1.0f)
		{
			*m_AnimationOffset = 2;
		}
		else if (m_Timer >= 1.0f)
		{
			*m_AnimationOffset = 0;
			m_Timer = 0.0f;
		}

		return;
	}
	*m_AnimationOffset = 0;
	m_Timer = 0.0;
}

World::TileTexture SpriteMap::directionRow(World::Direction direction, World::TileTexture textureOrigin)
{
	switch (direction)
	{
	case World::Direction::NORTH:
		textureOrigin.textureY += 3;
		return textureOrigin;
	case World::Direction::SOUTH:
		textureOrigin.textureY += 2;
		return textureOrigin;
	case World::Direction::EAST:
		return textureOrigin;
	case World::Direction::WEST:
		textureOrigin.textureY += 1;
		return textureOrigin;
	default:
		return textureOrigin;
	}
}

bool PlayerWalk::canWalk()
{
	std::vector<World::MovementPermissions>* permissions = World::Level::queryPermissions(*m_CurrentLevel);
	World::LevelDimensions dimensions = World::Level::queryDimensions(*m_CurrentLevel);
	unsigned int tileLookupX = *m_TileX;
	unsigned int tileLookupZ = *m_TileZ;

	//switch current direction, say what is next tile for each
	switch (*m_Direction)
	{
	case World::Direction::EAST:
		tileLookupX++;
		break;
	case World::Direction::WEST:
		tileLookupX--;
		break;
	case World::Direction::NORTH:
		tileLookupZ++;
		break;
	case World::Direction::SOUTH:
		tileLookupZ--;
		break;
	default:
		break;
	}

	//if outside level bounds, check current tile instead
	bool leavingLevel = false;
	if (tileLookupX < 0 || tileLookupX >= dimensions.levelW || tileLookupZ < 0 || tileLookupZ >= dimensions.levelH)
	{
		tileLookupX = *m_TileX;
		tileLookupZ = *m_TileZ;
		leavingLevel = true;
	}

	//Lookup permission for next tile
	unsigned int lookupIndex = tileLookupX * dimensions.levelH + tileLookupZ;
	World::MovementPermissions permission = World::Level::queryPermissions(*m_CurrentLevel)->at(lookupIndex);

	//Check if leaving level
	if (leavingLevel)
	{
		if (permission == World::MovementPermissions::LEVEL_BRIDGE)
		{
			return true;
		}
		return false;
	}
	
	switch (permission)
	{
	case World::MovementPermissions::WALL:
		return false;
	default:
		return true;
	}

	return false;
}

bool PlayerWalk::startWalk()
{
	*m_Walking = true;
	m_Timer = 0.0;
	return true;
}

const float PlayerWalk::s_ReverseAdjustment = 0.965f;
void PlayerWalk::update(double deltaTime) 
{
	//Checks to start walking
	bool anyHeld = false;
	if (!*m_Walking) 
	{
		if (*m_Up) 
		{
			*m_Direction = World::Direction::NORTH;
			if (canWalk()) 
			{
				anyHeld = startWalk();
			}
		}
		else if (*m_Down)
		{
			*m_Direction = World::Direction::SOUTH;
			if (canWalk())
			{
				anyHeld = startWalk();
			}
		}
		else if (*m_Left)
		{
			*m_Direction = World::Direction::WEST;
			if (canWalk())
			{
				anyHeld = startWalk();
			}
		}
		else if (*m_Right)
		{
			*m_Direction = World::Direction::EAST;
			if (canWalk())
			{
				anyHeld = startWalk();
			}
		}
	}

	if (m_Timer < 1.0 && *m_Walking) 
	{
		switch (*m_Direction)
		{
		case World::Direction::EAST:
			*m_XPos += deltaTime * World::TILE_SIZE;
			TranslateShape<TextureVertex>(m_Sprite, deltaTime * World::TILE_SIZE, 0.0f, 0.0f, Shape::QUAD);
			break;
		case World::Direction::WEST:
			*m_XPos += deltaTime * -World::TILE_SIZE * s_ReverseAdjustment;
			TranslateShape<TextureVertex>(m_Sprite, deltaTime * -World::TILE_SIZE * s_ReverseAdjustment, 0.0f, 0.0f, Shape::QUAD);
			break;
		case World::Direction::NORTH:
			*m_ZPos += deltaTime * -World::TILE_SIZE * s_ReverseAdjustment;
			TranslateShape<TextureVertex>(m_Sprite, 0.0f, 0.0f, deltaTime * -World::TILE_SIZE * s_ReverseAdjustment, Shape::QUAD);
			break;
		case World::Direction::SOUTH:
			*m_ZPos += deltaTime * World::TILE_SIZE;
			TranslateShape<TextureVertex>(m_Sprite, 0.0f, 0.0f, deltaTime * World::TILE_SIZE, Shape::QUAD);
			break;
		default:
			break;
		}

		m_Timer += deltaTime;
	}
	//Inherit update method
	TilePosition::update(deltaTime);
	if (m_Timer >= 1.0) 
	{
		if (!anyHeld)
		{
			*m_Walking = false;
		}
		m_Timer = 0.0;

		//Centre on x and y
		Component2f origin = World::Level::queryOrigin(*m_CurrentLevel);
		float expectedX = (float)(World::TILE_SIZE * *m_TileX) + origin.a;
		float expectedZ = (float)(-World::TILE_SIZE * *m_TileZ) + origin.b - World::TILE_SIZE / 2;
		float deltaX = *m_XPos - expectedX;
		float deltaZ = *m_ZPos - expectedZ;
		TranslateShape<TextureVertex>(m_Sprite, -deltaX, 0.0f, -deltaZ, Shape::QUAD);
		*m_XPos = expectedX;
		*m_ZPos = expectedZ;
	}
}
void PlayerAnimate::render()
{
	if (*m_Walking) 
	{
		return;
	}
	if (*m_Left)
	{
		*m_Direction = World::Direction::WEST;
	}
	else if (*m_Right) 
	{
		*m_Direction = World::Direction::EAST;
	}
	else if (*m_Up)
	{
		*m_Direction = World::Direction::NORTH;
	}
	else if (*m_Down)
	{
		*m_Direction = World::Direction::SOUTH;
	}
}

void PlayerCameraLock::render()
{
	m_Camera->setPos(*m_XPos + World::TILE_SIZE / 2, *m_YPos, *m_ZPos + m_Camera->height() / 8);
	m_Camera->moveUp(World::TILE_SIZE * 5);
}

//Objects
OverworldSprite::OverworldSprite(float xPos, float yPos, float zPos, float width, float height)
{
	//Set pos
	m_XPos = xPos; m_YPos = yPos; m_ZPos = zPos - World::TILE_SIZE / 2;
	//Make Sprite
	m_Sprite = CreateTextureQuad(xPos, yPos + height, width, height, 0.0f, 0.0f, 0.05f, 0.1f);
	//Position Sprite
	RotateShape<TextureVertex>((TextureVertex*)&m_Sprite, { m_XPos + width / 2, m_YPos, 0.0f }, -45.0f, Shape::QUAD, Axis::X);
	TranslateShape<TextureVertex>((TextureVertex*)&m_Sprite, 0.0f, 0.0f, m_ZPos, Shape::QUAD);
}

void OverworldSprite::setSprite(UVData data) 
{
	SetQuadUV((TextureVertex*)&m_Sprite, data.uvX, data.uvY, data.uvWidth, data.uvHeight);
}