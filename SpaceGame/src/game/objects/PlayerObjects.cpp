#include "game/objects/PlayerObjects.h"

//Player
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
	if (Ov_Translation::CheckCanWalk(m_PlayerData.get()))
	{
		//Unblock previous tile, block new tile - fix weird bug
		World::ModifyTilePerm(m_PlayerData->m_CurrentLevel, m_PlayerData->m_Direction, *m_Tile, m_PlayerData->m_WorldLevel, m_PlayerData->m_LastPermission, m_PlayerData->m_LastPermissionPtr);
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

	//Checks to start walking - don't start until turn after
	if (checkInputs())
	{
		return;
	}

	if (m_PlayerData->m_Walking)
	{
		Ov_Translation::SpriteWalk(m_PlayerData.get(), deltaTime);
	}
	else if (m_PlayerData->m_Running)
	{
		Ov_Translation::SpriteRun(m_PlayerData.get(), deltaTime);
	}
}

void PlayerCameraLock::render()
{
	m_Camera->setPos(*m_XPos, *m_YPos, *m_ZPos + m_Camera->height() / 8);
	m_Camera->moveUp(World::TILE_SIZE * 5);
}

void PlayerEncounter::update(double deltaTime)
{
	if (!(m_LastTile.x == (*m_Tile).x && m_LastTile.z == (*m_Tile).z))
	{
		m_LastTile = *m_Tile;
		switch (*m_Permission)
		{
		case World::MovementPermissions::TALL_GRASS:
			if (m_Random.next() < 115.0f)
			{
				*m_StartBattle = true;
			}
			break;
		default:
			break;
		}
	}
}

void UpdateGlobalLevel::update(double deltaTime)
{
	if (*m_GlobalPtr != *m_SpritePtr)
	{
		*m_GlobalPtr = *m_SpritePtr;
	}
}