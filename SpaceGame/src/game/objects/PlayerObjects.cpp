#include "game/objects/PlayerObjects.h"

//Player
bool PlayerMove::canWalk()
{
	World::Direction direction = m_PlayerData->m_Direction; World::LevelID levelID = m_PlayerData->m_CurrentLevel;
	World::WorldHeight worldLevel = m_PlayerData->m_WorldLevel;
	World::LevelPermission permissionNext = World::RetrievePermission(levelID, direction, { *m_TileX, *m_TileZ }, m_PlayerData->m_WorldLevel);
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

	//Check if next tile is impassable - check is here to prevent changing data like slope
	switch (permissionNext.perm)
	{
	case World::MovementPermissions::WALL:
		return false;
	case World::MovementPermissions::SPRITE_BLOCKING:
		return false;
	default:
		break;
	}

	//Check current permission if relevant
	switch (m_PlayerData->m_LastPermission)
	{
	case World::MovementPermissions::STAIRS_NORTH:
		m_CurrentIsSlope = true;
		if (direction == World::Direction::NORTH)
		{
			m_Ascend = 1;
		}
		else if (direction == World::Direction::SOUTH)
		{
			m_Ascend = -1;
			int levelTmp = (int)worldLevel;
			levelTmp--;
			m_PlayerData->m_WorldLevel = (World::WorldHeight)levelTmp;
		}
		break;
	case World::MovementPermissions::STAIRS_SOUTH:
		m_CurrentIsSlope = true;
		if (direction == World::Direction::SOUTH)
		{
			m_Ascend = 1;
		}
		else if (direction == World::Direction::NORTH)
		{
			m_Ascend = -1;
			int levelTmp = (int)worldLevel;
			levelTmp--;
			m_PlayerData->m_WorldLevel = (World::WorldHeight)levelTmp;
		}
		break;
	case World::MovementPermissions::STAIRS_EAST:
		m_CurrentIsSlope = true;
		if (direction == World::Direction::EAST)
		{
			m_Ascend = 1;
		}
		else if (direction == World::Direction::WEST)
		{
			m_Ascend = -1;
			int levelTmp = (int)worldLevel;
			levelTmp--;
			m_PlayerData->m_WorldLevel = (World::WorldHeight)levelTmp;
		}
		break;
	case World::MovementPermissions::STAIRS_WEST:
		m_CurrentIsSlope = true;
		if (direction == World::Direction::WEST)
		{
			m_Ascend = 1;
		}
		else if (direction == World::Direction::EAST)
		{
			m_Ascend = -1;
			int levelTmp = (int)worldLevel;
			levelTmp--;
			m_PlayerData->m_WorldLevel = (World::WorldHeight)levelTmp;
		}
		break;
	}

	//Check next permission if relevant
	switch (permissionNext.perm)
	{
	case World::MovementPermissions::STAIRS_NORTH:
	{
		m_NextIsSlope = true;
		if (direction == World::Direction::NORTH)
		{
			m_Ascend = 1;
			int levelTmp = (int)worldLevel;
			levelTmp++;
			m_PlayerData->m_WorldLevel = (World::WorldHeight)levelTmp;
			return true;
		}
		else if (direction == World::Direction::SOUTH)
		{
			m_Ascend = -1;
			return true;
		}
		return false;
	}
	case World::MovementPermissions::STAIRS_SOUTH:
	{
		m_NextIsSlope = true;
		if (direction == World::Direction::SOUTH)
		{
			m_Ascend = 1;
			int levelTmp = (int)worldLevel;
			levelTmp++;
			m_PlayerData->m_WorldLevel = (World::WorldHeight)levelTmp;
			return true;
		}
		else if (direction == World::Direction::NORTH)
		{
			m_Ascend = -1;
			return true;
		}
		return false;
	}
	case World::MovementPermissions::STAIRS_EAST:
	{
		m_NextIsSlope = true;
		if (direction == World::Direction::EAST)
		{
			m_Ascend = 1;
			int levelTmp = (int)worldLevel;
			levelTmp++;
			m_PlayerData->m_WorldLevel = (World::WorldHeight)levelTmp;
			return true;
		}
		else if (direction == World::Direction::WEST)
		{
			m_Ascend = -1;
			return true;
		}
		return false;
	}
	case World::MovementPermissions::STAIRS_WEST:
	{
		m_NextIsSlope = true;
		if (direction == World::Direction::WEST)
		{
			m_Ascend = 1;
			int levelTmp = (int)worldLevel;
			levelTmp++;
			m_PlayerData->m_WorldLevel = (World::WorldHeight)levelTmp;
			return true;
		}
		else if (direction == World::Direction::EAST)
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
	m_PlayerData->m_Walking = true;
	m_Timer = 0.0;
	return true;
}

bool PlayerMove::startRun()
{
	m_PlayerData->m_Running = true;
	m_Timer = 0.0;
	return true;
}

bool PlayerMove::walkPermHelper()
{
	if (canWalk())
	{
		//Unblock previous tile, block new tile - fix weird bug
		World::ModifyTilePerm(m_PlayerData->m_CurrentLevel, m_PlayerData->m_Direction, { *m_TileX, *m_TileZ }, m_PlayerData->m_WorldLevel, m_PlayerData->m_LastPermission, m_PlayerData->m_LastPermissionPtr);
		if (m_Input->HELD_SHIFT)
		{
			return startRun();
		}
		return startWalk();
	}
	return false;
}

bool PlayerMove::checkInputs()
{
	if (!m_PlayerData->m_Walking && !m_PlayerData->m_Running)
	{
		if (m_Input->HELD_W)
		{
			m_PlayerData->m_Direction = World::Direction::NORTH;
			return walkPermHelper();
		}
		else if (m_Input->HELD_S)
		{
			m_PlayerData->m_Direction = World::Direction::SOUTH;
			return walkPermHelper();
		}
		else if (m_Input->HELD_A)
		{
			m_PlayerData->m_Direction = World::Direction::WEST;
			return walkPermHelper();
		}
		else if (m_Input->HELD_D)
		{
			m_PlayerData->m_Direction = World::Direction::EAST;
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
	if (m_Input->PRESSED_W)
	{
		m_PlayerData->m_Direction = World::Direction::NORTH;
		return;
	}
	else if (m_Input->PRESSED_S)
	{
		m_PlayerData->m_Direction = World::Direction::SOUTH;
		return;
	}
	else if (m_Input->PRESSED_A)
	{
		m_PlayerData->m_Direction = World::Direction::WEST;
		return;
	}
	else if (m_Input->PRESSED_D)
	{
		m_PlayerData->m_Direction = World::Direction::EAST;
		return;
	}
}

void PlayerMove::update(double deltaTime) 
{
	//Inherit update method
	TilePosition::update(deltaTime);
	bool walking = m_PlayerData->m_Walking; bool running = m_PlayerData->m_Running;
	if (m_PlayerData->m_Controlled)
	{
		return;
	}
	else if (m_PlayerData->m_Busy && !(walking || running))
	{
		//If busy and not walking or running, don't update player
		return;
	}

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
	if (m_Timer < World::WALK_DURATION && walking) 
	{
		Ov_Translation::Walk(&m_PlayerData->m_Direction, m_XPos, m_ZPos, &m_PlayerData->m_Sprite, deltaTime, &m_Timer);
		if (m_Ascend == 1)
		{
			if (m_CurrentIsSlope && m_NextIsSlope)
			{
				Ov_Translation::AscendSlope(&m_PlayerData->m_YPos, &m_PlayerData->m_Sprite, deltaTime, false);
			}
			else
			{
				Ov_Translation::AscendSlope(&m_PlayerData->m_YPos, &m_PlayerData->m_Sprite, deltaTime, false, m_Timer, m_MoveVerticalFirst);
			}
		}
		else if (m_Ascend == -1)
		{
			if (m_CurrentIsSlope && m_NextIsSlope)
			{
				Ov_Translation::DescendSlope(&m_PlayerData->m_YPos, &m_PlayerData->m_Sprite, deltaTime, false);
			}
			else
			{
				Ov_Translation::DescendSlope(&m_PlayerData->m_YPos, &m_PlayerData->m_Sprite, deltaTime, false, m_Timer, m_MoveVerticalFirst);
			}
		}
	}
	//Run method
	else if (m_Timer < World::RUN_DURATION && running)
	{
		Ov_Translation::Run(&m_PlayerData->m_Direction, m_XPos, m_ZPos, &m_PlayerData->m_Sprite, deltaTime, &m_Timer);
		if (m_Ascend == 1)
		{
			if (m_CurrentIsSlope && m_NextIsSlope)
			{
				Ov_Translation::AscendSlope(&m_PlayerData->m_YPos, &m_PlayerData->m_Sprite, deltaTime, true);
			}
			else
			{
				Ov_Translation::AscendSlope(&m_PlayerData->m_YPos, &m_PlayerData->m_Sprite, deltaTime, true, m_Timer, m_MoveVerticalFirst);
			}
		}
		else if (m_Ascend == -1)
		{
			if (m_CurrentIsSlope && m_NextIsSlope)
			{
				Ov_Translation::DescendSlope(&m_PlayerData->m_YPos, &m_PlayerData->m_Sprite, deltaTime, true);
			}
			else
			{
				Ov_Translation::DescendSlope(&m_PlayerData->m_YPos, &m_PlayerData->m_Sprite, deltaTime, true, m_Timer, m_MoveVerticalFirst);
			}
		}
	}

	//If at end of run or walk cycle
	if ((m_Timer >= World::WALK_DURATION && walking) || (m_Timer >= World::RUN_DURATION && running))
	{
		cycleEnd(anyHeld);
	}
}

void PlayerMove::cycleEnd(bool anyHeld)
{
	if (!anyHeld)
	{
		m_PlayerData->m_Walking = false;
		m_PlayerData->m_Running = false;
	}
	m_Timer = 0.0;

	//Centre on x and y
	if (!m_NextIsSlope)
	{
		//Is false as 
		Ov_Translation::CentreOnTile(m_PlayerData->m_CurrentLevel, m_PlayerData->m_WorldLevel, m_XPos, &m_PlayerData->m_YPos, m_ZPos, *m_TileX, *m_TileZ, &m_PlayerData->m_Sprite, false);
		return;
	}
	if (m_NextIsSlope)
	{
		//Is true as 
		Ov_Translation::CentreOnTile(m_PlayerData->m_CurrentLevel, m_PlayerData->m_WorldLevel, m_XPos, &m_PlayerData->m_YPos, m_ZPos, *m_TileX, *m_TileZ, &m_PlayerData->m_Sprite, true);
		return;
	}

}

void PlayerCameraLock::render()
{
	m_Camera->setPos(*m_XPos, *m_YPos, *m_ZPos + m_Camera->height() / 8);
	m_Camera->moveUp(World::TILE_SIZE * 5);
}

void UpdateGlobalLevel::update(double deltaTime)
{
	if (*m_GlobalPtr != *m_SpritePtr)
	{
		*m_GlobalPtr = *m_SpritePtr;
	}
}