#include "game/objects/PlayerObjects.h"

//Player
bool PlayerMove::canWalk()
{
	World::RetrievePermission permission = World::retrievePermission(*m_CurrentLevel, *m_Direction, { *m_TileX, *m_TileZ });

	//Check if leaving level
	if (permission.leaving)
	{
		if (permission.perm == World::MovementPermissions::LEVEL_BRIDGE)
		{
			return true;
		}
		return false;
	}
	
	switch (permission.perm)
	{
	case World::MovementPermissions::WALL:
		return false;
	case World::MovementPermissions::SPRITE_BLOCKING:
		return false;
	default:
		return true;
	}
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

void PlayerMove::modifyTilePerm()
{
	//Get level data
	World::LevelDimensions dim = World::Level::queryDimensions(*m_CurrentLevel);
	std::vector<World::MovementPermissions>* perm = World::Level::queryPermissions(*m_CurrentLevel);
	
	//Clear tile leaving
	unsigned int index = *m_TileX * dim.levelH + *m_TileZ;
	perm->at(index) = World::MovementPermissions::CLEAR;

	//Block tile entering
	World::TileLoc tileLookup = World::NextTileInInputDirection(*m_Direction, { *m_TileX, *m_TileZ });
	index = tileLookup.x * dim.levelH + tileLookup.z;
	perm->at(index) = World::MovementPermissions::SPRITE_BLOCKING;
}

bool PlayerMove::walkPermHelper()
{
	if (canWalk())
	{
		//Unblock previous tile, block new tile
		World::ModifyTilePerm(*m_CurrentLevel, *m_Direction, { *m_TileX, *m_TileZ });
		if (*m_Shift)
		{
			return startRun();
		}
		return startWalk();
	}
	return false;
}

bool PlayerMove::checkInputs()
{
	if (!*m_Walking && !*m_Running)
	{
		if (*m_Up)
		{
			*m_Direction = World::Direction::NORTH;
			return walkPermHelper();
		}
		else if (*m_Down)
		{
			*m_Direction = World::Direction::SOUTH;
			return walkPermHelper();
		}
		else if (*m_Left)
		{
			*m_Direction = World::Direction::WEST;
			return walkPermHelper();
		}
		else if (*m_Right)
		{
			*m_Direction = World::Direction::EAST;
			return walkPermHelper();
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

void PlayerMove::cycleEnd(bool anyHeld)
{
	if (!anyHeld)
	{
		*m_Walking = false;
		*m_Running = false;
	}
	m_Timer = 0.0;

	//Centre on x and y
	Ov_Translation::CentreOnTile(*m_CurrentLevel, m_XPos, m_ZPos, *m_TileX, *m_TileZ, m_Sprite);
}

void PlayerMove::update(double deltaTime) 
{

	//Checks to start walking
	bool anyHeld = checkInputs();

	//Walk method
	if (m_Timer < World::WALK_DURATION && *m_Walking) 
	{
		Ov_Translation::Walk(m_Direction, m_XPos, m_ZPos, m_Sprite, deltaTime, &m_Timer);
	}
	//Run method
	else if (m_Timer < World::RUN_DURATION && *m_Running)
	{
		Ov_Translation::Run(m_Direction, m_XPos, m_ZPos, m_Sprite, deltaTime, &m_Timer);
	}

	//Inherit update method
	TilePosition::update(deltaTime);

	//If at end of run or walk cycle
	if ((m_Timer >= World::WALK_DURATION && *m_Walking) || (m_Timer >= World::RUN_DURATION && *m_Running))
	{
		cycleEnd(anyHeld);
	}
}

void PlayerCameraLock::render()
{
	m_Camera->setPos(*m_XPos, *m_YPos, *m_ZPos + m_Camera->height() / 8);
	m_Camera->moveUp(World::TILE_SIZE * 5);
}