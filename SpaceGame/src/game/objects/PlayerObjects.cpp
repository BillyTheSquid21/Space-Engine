#include "game/objects/PlayerObjects.h"

//Player
bool PlayerMove::canWalk()
{
	World::LevelPermission permissionNext = World::RetrievePermission(*m_CurrentLevel, *m_Direction, { *m_TileX, *m_TileZ });
	World::LevelPermission permissionCurrent = World::RetrievePermission(*m_CurrentLevel, { *m_TileX, *m_TileZ });
	m_Ascend = 0; m_CurrentIsSlope = false; m_NextIsSlope = false;

	//Check if leaving level
	if (permissionNext.leaving)
	{
		if (permissionNext.perm == World::MovementPermissions::LEVEL_BRIDGE)
		{
			return true;
		}
		return false;
	}

	//Check current permission if relevant
	switch (permissionCurrent.perm)
	{
	case World::MovementPermissions::STAIRS_NORTH:
		m_CurrentIsSlope = true;
		if (*m_Direction == World::Direction::NORTH)
		{
			m_Ascend = 1;
		}
		else if (*m_Direction == World::Direction::SOUTH)
		{
			m_Ascend = -1;
			int levelTmp = (int)*m_WorldLevel;
			levelTmp--;
			*m_WorldLevel = (World::WorldLevel)levelTmp;
		}
		break;
	case World::MovementPermissions::STAIRS_SOUTH:
		m_CurrentIsSlope = true;
		if (*m_Direction == World::Direction::SOUTH)
		{
			m_Ascend = 1;
		}
		else if (*m_Direction == World::Direction::NORTH)
		{
			m_Ascend = -1;
			int levelTmp = (int)*m_WorldLevel;
			levelTmp--;
			*m_WorldLevel = (World::WorldLevel)levelTmp;
		}
		break;
	case World::MovementPermissions::STAIRS_EAST:
		m_CurrentIsSlope = true;
		if (*m_Direction == World::Direction::EAST)
		{
			m_Ascend = 1;
		}
		else if (*m_Direction == World::Direction::WEST)
		{
			m_Ascend = -1;
			int levelTmp = (int)*m_WorldLevel;
			levelTmp--;
			*m_WorldLevel = (World::WorldLevel)levelTmp;
		}
		break;
	case World::MovementPermissions::STAIRS_WEST:
		m_CurrentIsSlope = true;
		if (*m_Direction == World::Direction::WEST)
		{
			m_Ascend = 1;
		}
		else if (*m_Direction == World::Direction::EAST)
		{
			m_Ascend = -1;
			int levelTmp = (int)*m_WorldLevel;
			levelTmp--;
			*m_WorldLevel = (World::WorldLevel)levelTmp;
		}
		break;
	}
	
	//Check next permission if relevant
	switch (permissionNext.perm)
	{
	case World::MovementPermissions::WALL:
		return false;
	case World::MovementPermissions::SPRITE_BLOCKING:
		return false;
	case World::MovementPermissions::STAIRS_NORTH:
	{
		m_NextIsSlope = true;
		if (*m_Direction == World::Direction::NORTH)
		{
			m_Ascend = 1;
			int levelTmp = (int)*m_WorldLevel;
			levelTmp++;
			*m_WorldLevel = (World::WorldLevel)levelTmp;
			return true;
		}
		else if (*m_Direction == World::Direction::SOUTH)
		{
			m_Ascend = -1;
			return true;
		}
		return false;
	}
	case World::MovementPermissions::STAIRS_SOUTH:
	{
		m_NextIsSlope = true;
		if (*m_Direction == World::Direction::SOUTH)
		{
			m_Ascend = 1;
			int levelTmp = (int)*m_WorldLevel;
			levelTmp++;
			*m_WorldLevel = (World::WorldLevel)levelTmp;
			return true;
		}
		else if (*m_Direction == World::Direction::NORTH)
		{
			m_Ascend = -1;
			return true;
		}
		return false;
	}
	case World::MovementPermissions::STAIRS_EAST:
	{
		m_NextIsSlope = true;
		if (*m_Direction == World::Direction::EAST)
		{
			m_Ascend = 1;
			int levelTmp = (int)*m_WorldLevel;
			levelTmp++;
			*m_WorldLevel = (World::WorldLevel)levelTmp;
			return true;
		}
		else if (*m_Direction == World::Direction::WEST)
		{
			m_Ascend = -1;
			return true;
		}
		return false;
	}
	case World::MovementPermissions::STAIRS_WEST:
	{
		m_NextIsSlope = true;
		if (*m_Direction == World::Direction::WEST)
		{
			m_Ascend = 1;
			int levelTmp = (int)*m_WorldLevel;
			levelTmp++;
			*m_WorldLevel = (World::WorldLevel)levelTmp;
			return true;
		}
		else if (*m_Direction == World::Direction::EAST)
		{
			m_Ascend = -1;
			return true;
		}
		return false;
	}
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
	if (!m_NextIsSlope)
	{
		//Is false as 
		Ov_Translation::CentreOnTile(*m_CurrentLevel, *m_WorldLevel, m_XPos, m_YPos, m_ZPos, *m_TileX, *m_TileZ, m_Sprite, false);
		return;
	}
	if (m_NextIsSlope)
	{
		//Is true as 
		Ov_Translation::CentreOnTile(*m_CurrentLevel, *m_WorldLevel, m_XPos, m_YPos, m_ZPos, *m_TileX, *m_TileZ, m_Sprite, true);
		return;
	}
	
}

void PlayerMove::update(double deltaTime) 
{
	//Checks to start walking
	bool anyHeld = checkInputs();

	//Check how to ascend
	m_MoveVerticalFirst = false;
	if (m_Ascend)
	{
		if (m_CurrentIsSlope && !m_NextIsSlope)
		{
			m_MoveVerticalFirst = true;
		}
		if (!m_CurrentIsSlope && m_NextIsSlope)
		{
			m_MoveVerticalFirst = false;
		}
	}

	//Walk method
	if (m_Timer < World::WALK_DURATION && *m_Walking) 
	{
		Ov_Translation::Walk(m_Direction, m_XPos, m_ZPos, m_Sprite, deltaTime, &m_Timer);
		if (m_Ascend == 1)
		{
			if (m_CurrentIsSlope && m_NextIsSlope)
			{
				Ov_Translation::AscendSlope(m_YPos, m_Sprite, deltaTime, false);
			}
			else
			{
				Ov_Translation::AscendSlope(m_YPos, m_Sprite, deltaTime, false, m_Timer, m_MoveVerticalFirst);
			}
		}
		else if (m_Ascend == -1)
		{
			if (m_CurrentIsSlope && m_NextIsSlope)
			{
				Ov_Translation::DescendSlope(m_YPos, m_Sprite, deltaTime, false);
			}
			else
			{
				Ov_Translation::DescendSlope(m_YPos, m_Sprite, deltaTime, false, m_Timer, m_MoveVerticalFirst);
			}
		}
	}
	//Run method
	else if (m_Timer < World::RUN_DURATION && *m_Running)
	{
		Ov_Translation::Run(m_Direction, m_XPos, m_ZPos, m_Sprite, deltaTime, &m_Timer);
		if (m_Ascend == 1)
		{
			if (m_CurrentIsSlope && m_NextIsSlope)
			{
				Ov_Translation::AscendSlope(m_YPos, m_Sprite, deltaTime, true);
			}
			else
			{
				Ov_Translation::AscendSlope(m_YPos, m_Sprite, deltaTime, true, m_Timer, m_MoveVerticalFirst);
			}
		}
		else if (m_Ascend == -1)
		{
			if (m_CurrentIsSlope && m_NextIsSlope)
			{
				Ov_Translation::DescendSlope(m_YPos, m_Sprite, deltaTime, true);
			}
			else
			{
				Ov_Translation::DescendSlope(m_YPos, m_Sprite, deltaTime, true, m_Timer, m_MoveVerticalFirst);
			}
		}
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