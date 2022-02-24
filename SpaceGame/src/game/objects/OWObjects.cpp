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
	float deltaY = *m_YPos - origin.b;
	*m_TileX = (unsigned int)deltaX / World::TILE_SIZE;
	*m_TileY = (unsigned int)deltaY / World::TILE_SIZE;
}

void PlayerWalk::update(double deltaTime) 
{
	//Inherit update method
	TilePosition::update(deltaTime);

	//Checks to start walking
	if (!m_Walking) 
	{
		if (*m_Up) 
		{
			*m_Direction = World::Direction::NORTH;
			*m_Walking = true;
			m_Timer = 0.0;
		}
		else if (*m_Down)
		{
			*m_Direction = World::Direction::SOUTH;
			*m_Walking = true;
			m_Timer = 0.0;
		}
		else if (*m_Left)
		{
			*m_Direction = World::Direction::WEST;
			*m_Walking = true;
			m_Timer = 0.0;
		}
		else if (*m_Right)
		{
			*m_Direction = World::Direction::EAST;
			*m_Walking = true;
			m_Timer = 0.0;
		}
	}
}

//Objects
OverworldSprite::OverworldSprite(float xPos, float yPos, float zPos, float width, float height) 
{
	//Set pos
	m_XPos = xPos; m_YPos = yPos; m_ZPos = zPos + World::TILE_SIZE / 2;
	//Make Sprite
	m_Sprite = CreateTextureQuad(xPos, yPos + height, width, height, 0.0f, 0.0f, 0.05f, 0.1f);
	//Position Sprite
	RotateShape<TextureVertex>((TextureVertex*)&m_Sprite, { m_XPos + width / 2, m_YPos, 0.0f }, -45.0f, Shape::QUAD, Axis::X);
	TranslateShape<TextureVertex>((TextureVertex*)&m_Sprite, 0.0f, 0.0f, -m_ZPos, Shape::QUAD);
}