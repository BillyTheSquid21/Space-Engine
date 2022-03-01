#include "game/objects/PlayerObjects.h"

//Player
bool PlayerMove::canWalk()
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

bool PlayerMove::startWalk()
{
	*m_Walking = true;
	m_Timer = 0.0;
	return true;
}

bool PlayerMove::startRun()
{
	*m_Running = true;
	m_Timer = 0.0;
	return true;
}

bool PlayerMove::checkInputs()
{
	if (!*m_Walking && !*m_Running)
	{
		if (*m_Up)
		{
			*m_Direction = World::Direction::NORTH;
			if (canWalk())
			{
				if (*m_Shift)
				{
					return startRun();
				}
				return startWalk();
			}
		}
		else if (*m_Down)
		{
			*m_Direction = World::Direction::SOUTH;
			if (canWalk())
			{
				if (*m_Shift)
				{
					return startRun();
				}
				return startWalk();
			}
		}
		else if (*m_Left)
		{
			*m_Direction = World::Direction::WEST;
			if (canWalk())
			{
				if (*m_Shift)
				{
					return startRun();
				}
				return startWalk();
			}
		}
		else if (*m_Right)
		{
			*m_Direction = World::Direction::EAST;
			if (canWalk())
			{
				if (*m_Shift)
				{
					return startRun();
				}
				return startWalk();
			}
		}
		//Check if player changes direction with no move
		faceDirection();
	}

	return false;
}

void PlayerMove::faceDirection()
{
	//if isnt held, check for pressed
	if (*m_UpSingle)
	{
		*m_Direction = World::Direction::NORTH;
		return;
	}
	else if (*m_DownSingle)
	{
		*m_Direction = World::Direction::SOUTH;
		return;
	}
	else if (*m_LeftSingle)
	{
		*m_Direction = World::Direction::WEST;
		return;
	}
	else if (*m_RightSingle)
	{
		*m_Direction = World::Direction::EAST;
		return;
	}
}

void PlayerMove::walk(double deltaTime)
{
	switch (*m_Direction)
	{
	case World::Direction::EAST:
		*m_XPos += deltaTime * World::TILE_SIZE;
		TranslateShape<TextureVertex>(m_Sprite, deltaTime * World::TILE_SIZE, 0.0f, 0.0f, Shape::QUAD);
		break;
	case World::Direction::WEST:
		*m_XPos += deltaTime * -World::TILE_SIZE;
		TranslateShape<TextureVertex>(m_Sprite, deltaTime * -World::TILE_SIZE, 0.0f, 0.0f, Shape::QUAD);
		break;
	case World::Direction::NORTH:
		*m_ZPos += deltaTime * -World::TILE_SIZE;
		TranslateShape<TextureVertex>(m_Sprite, 0.0f, 0.0f, deltaTime * -World::TILE_SIZE, Shape::QUAD);
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

void PlayerMove::run(double deltaTime)
{
	switch (*m_Direction)
	{
	case World::Direction::EAST:
		*m_XPos += deltaTime * World::TILE_SIZE * 2.5f;
		TranslateShape<TextureVertex>(m_Sprite, deltaTime * World::TILE_SIZE * 2.5f, 0.0f, 0.0f, Shape::QUAD);
		break;
	case World::Direction::WEST:
		*m_XPos += deltaTime * -World::TILE_SIZE * 2.5f;
		TranslateShape<TextureVertex>(m_Sprite, deltaTime * -World::TILE_SIZE * 2.5f, 0.0f, 0.0f, Shape::QUAD);
		break;
	case World::Direction::NORTH:
		*m_ZPos += deltaTime * -World::TILE_SIZE * 2.5f;
		TranslateShape<TextureVertex>(m_Sprite, 0.0f, 0.0f, deltaTime * -World::TILE_SIZE * 2.5f, Shape::QUAD);
		break;
	case World::Direction::SOUTH:
		*m_ZPos += deltaTime * World::TILE_SIZE * 2.5f;
		TranslateShape<TextureVertex>(m_Sprite, 0.0f, 0.0f, deltaTime * World::TILE_SIZE * 2.5f, Shape::QUAD);
		break;
	default:
		break;
	}

	m_Timer += deltaTime;
}

void PlayerMove::cycleEnd(bool anyHeld)
{
	if (!anyHeld)
	{
		*m_Walking = false;
		*m_Running = false;
	}
	m_Timer = 0.0;

	//Centre on x and y
	Component2f origin = World::Level::queryOrigin(*m_CurrentLevel);
	float expectedX = (float)(World::TILE_SIZE * *m_TileX) + origin.a + World::TILE_SIZE / 2;
	float expectedZ = (float)(-World::TILE_SIZE * *m_TileZ) + origin.b - World::TILE_SIZE / 2;
	float deltaX = *m_XPos - expectedX;
	float deltaZ = *m_ZPos - expectedZ;
	TranslateShape<TextureVertex>(m_Sprite, -deltaX, 0.0f, -deltaZ, Shape::QUAD);
	*m_XPos = expectedX;
	*m_ZPos = expectedZ;
}

void PlayerMove::update(double deltaTime) 
{

	//Checks to start walking
	bool anyHeld = checkInputs();

	//Walk method
	if (m_Timer < 1.0 && *m_Walking) 
	{
		walk(deltaTime);
	}
	//Run method
	else if (m_Timer < 0.4 && *m_Running)
	{
		run(deltaTime);
	}

	//Inherit update method
	TilePosition::update(deltaTime);

	//If at end of run or walk cycle
	if ((m_Timer >= 1.0 && *m_Walking) || (m_Timer >= 0.4 && *m_Running))
	{
		cycleEnd(anyHeld);
	}
}

void PlayerCameraLock::render()
{
	m_Camera->setPos(*m_XPos + World::TILE_SIZE / 2, *m_YPos, *m_ZPos + m_Camera->height() / 8);
	m_Camera->moveUp(World::TILE_SIZE * 5);
}