#include "game/objects/PokemonFollow.h"

void PokemonFollow::update(double deltaTime)
{
	using namespace Geometry; using namespace SGRender;
	TilePosition::update(deltaTime);

	//Check if entering pointer mode
	if (m_Input->PRESSED_CTRL && m_Cooldown == 0)
	{
		if (!m_Player->m_Busy)
		{
			if (!m_PtrMode)
			{
				m_PtrMode = true;
				m_Player->m_Busy = true;
				m_Pointer->messageAllRender((int)SGObject::Message::ACTIVATE);
				m_Cooldown = 0.5;

				//Put under pokemon
				Struct3f pos = { m_Pointer->m_XPos, m_Pointer->m_YPos, m_Pointer->m_ZPos };

				//New pos
				m_Pointer->m_XPos = m_Pkm->m_XPos; m_Pointer->m_YPos = m_Pkm->m_YPos;
				m_Pointer->m_ZPos = m_Pkm->m_ZPos;

				//Level
				m_Pointer->m_WorldLevel = m_Pkm->m_WorldLevel;
				m_Pointer->m_YPos = (((float)m_Pointer->m_WorldLevel / sqrt(2)) * World::TILE_SIZE);

				Struct3f newPos = { m_Pointer->m_XPos, m_Pointer->m_YPos, m_Pointer->m_ZPos };

				//Translate sprite
				Position<NTVertex>(&m_Pointer->m_Sprite, pos, newPos, Shape::QUAD);
			
				m_LastPkmTile = m_Pkm->m_Tile;
			}
		}
		else
		{
			if (m_PtrMode && m_Script->getInstruction() != ScriptInstruction::PLAYER_RUN_TO_TILE)
			{
				m_PtrMode = false;
				m_Player->m_Busy = false;
				m_Pointer->messageAllRender((int)SGObject::Message::DEACTIVATE);
				m_Cooldown = 0.5;

				//Check if are where meant to be
				if (m_Pkm->m_Tile != m_LastPkmTile)
				{
					//Create script to walk there
					Script script(new ScriptElement[2]);
					script[0].instruction = ScriptInstruction::PLAYER_RUN_TO_TILE;
					script[0].info.tileInfo.x = m_LastPkmTile.x;
					script[0].info.tileInfo.z = m_LastPkmTile.z;
					m_Script->setScript(script, 2);
				}
			}
		}
		m_Input->clear();
	}

	//Move selector around
	if (m_PtrMode)
	{
		if (m_Input->anyHeld() && m_Cooldown == 0)
		{
			Struct3f pos = { m_Pointer->m_XPos, m_Pointer->m_YPos, m_Pointer->m_ZPos };
			Struct3f newPos;
			if (m_Input->PRESSED_W)
			{
				if (checkPerm(World::Direction::NORTH, pos))
				{
					m_Pointer->m_ZPos -= World::TILE_SIZE;
					Struct3f newPos = { m_Pointer->m_XPos, m_Pointer->m_YPos, m_Pointer->m_ZPos };
					Position<NTVertex>(&m_Pointer->m_Sprite, pos, newPos, Shape::QUAD);
					m_Cooldown = 0.2;
				}
			}
			else if (m_Input->PRESSED_S)
			{
				if (checkPerm(World::Direction::SOUTH, pos))
				{
					m_Pointer->m_ZPos += World::TILE_SIZE;
					Struct3f newPos = { m_Pointer->m_XPos, m_Pointer->m_YPos, m_Pointer->m_ZPos };
					Position<NTVertex>(&m_Pointer->m_Sprite, pos, newPos, Shape::QUAD);
					m_Cooldown = 0.2;
				}
			}
			else if (m_Input->PRESSED_A)
			{
				if (checkPerm(World::Direction::WEST, pos))
				{
					m_Pointer->m_XPos -= World::TILE_SIZE;
					Struct3f newPos = { m_Pointer->m_XPos, m_Pointer->m_YPos, m_Pointer->m_ZPos };
					Position<NTVertex>(&m_Pointer->m_Sprite, pos, newPos, Shape::QUAD);
					m_Cooldown = 0.2;
				}
			}
			else if (m_Input->PRESSED_D)
			{
				if (checkPerm(World::Direction::EAST, pos))
				{
					m_Pointer->m_XPos += World::TILE_SIZE;
					Struct3f newPos = { m_Pointer->m_XPos, m_Pointer->m_YPos, m_Pointer->m_ZPos };
					Position<NTVertex>(&m_Pointer->m_Sprite, pos, newPos, Shape::QUAD);
					m_Cooldown = 0.2;
				}
			}
			else if (m_Input->PRESSED_E)
			{
				//Create script to walk there
				Script script(new ScriptElement[2]);
				script[0].instruction = ScriptInstruction::PLAYER_RUN_TO_TILE;
				script[0].info.tileInfo.x = m_Pointer->m_Tile.x;
				script[0].info.tileInfo.z = m_Pointer->m_Tile.z;
				script[0].info.tileInfo.h = (int8_t)m_Pointer->m_WorldLevel;
				PathFinding::Path path = PathFinding::GetPath(m_Pkm, m_Pointer->m_Tile, m_Pointer->m_WorldLevel);
				m_Script->setPath(path);
				m_Script->setScript(script, 2);
				m_Cooldown = 0.2;
			}
		}
	}
	else
	{
		ScriptElement& el = m_Script->getElement(0);
		if (el.info.tileInfo.x != m_LastDestTile.x && el.info.tileInfo.z != m_LastDestTile.z)
		{
			el.info.tileInfo.x = m_LastDestTile.x; el.info.tileInfo.z = m_LastDestTile.z; el.info.tileInfo.h = (int8_t)m_Player->m_WorldLevel;
		}
	}

	//Cooldown for selections
	if (m_Cooldown != 0.0)
	{
		m_Cooldown -= deltaTime;
		if (m_Cooldown <= 0.0)
		{
			m_Cooldown = 0.0;
		}
	}

	if (m_Script->getInstruction() == ScriptInstruction::PLAYER_RUN_TO_TILE)
	{
		return;
	}

	if (!m_WasWalking && m_Player->m_Walking)
	{
		m_WasWalking = true;

		//Get direction to new tile
		World::Direction dir = World::DirectionOfAdjacentTile(*m_Tile, m_LastDestTile);
		m_Pkm->m_Direction = dir;

		//Check where new tile is relatively - if suddenly changes just move in dir of player
		World::Tile tileDiff = m_Player->m_Tile - m_LastDestTile;
		if (tileDiff.x > 1 || tileDiff.x < -1 || tileDiff.z > 1 || tileDiff.z < -1)
		{
			m_Pkm->m_Direction = m_Player->m_Direction;
			m_Pkm->m_CurrentLevel = m_Player->m_CurrentLevel;
			m_Pointer->m_CurrentLevel = m_Player->m_CurrentLevel;
		}

		//Create script to walk there
		Script script(new ScriptElement[1]);
		script[0].instruction = ScriptInstruction::PLAYER_WALK;
		m_Script->setScript(script, 1);
	}
	else if (!m_WasRunning && m_Player->m_Running)
	{
		m_WasRunning = true;

		//Get direction to new tile
		World::Direction dir = World::DirectionOfAdjacentTile(*m_Tile, m_LastDestTile);
		m_Pkm->m_Direction = dir;

		//Check where new tile is relatively - if suddenly changes just move in dir of player
		World::Tile tileDiff = m_Player->m_Tile - m_LastDestTile;
		if (tileDiff.x > 1 || tileDiff.x < -1 || tileDiff.z > 1 || tileDiff.z < -1)
		{
			m_Pkm->m_Direction = m_Player->m_Direction;
			m_Pkm->m_CurrentLevel = m_Player->m_CurrentLevel;
			m_Pointer->m_CurrentLevel = m_Player->m_CurrentLevel;
		}

		//Create script to walk there
		Script script(new ScriptElement[1]);
		script[0].instruction = ScriptInstruction::PLAYER_RUN;
		m_Script->setScript(script, 1);
	}
	else if (m_WasWalking && !(m_Pkm->m_Walking))
	{
		m_LastDestTile = m_Player->m_Tile;

		//Change direction to face player
		World::Direction dir = World::DirectionOfAdjacentTile(m_Pkm->m_Tile, m_LastDestTile);
		m_Pkm->m_Direction = dir;
		m_WasWalking = false;
	}
	else if (m_WasRunning && !(m_Pkm->m_Running))
	{
		m_LastDestTile = m_Player->m_Tile;

		//Change direction to face player
		World::Direction dir = World::DirectionOfAdjacentTile(m_Pkm->m_Tile, m_LastDestTile);
		m_Pkm->m_Direction = dir;
		m_WasRunning = false;
	}
}

static void IncLevel(Ov_Sprite::Sprite* ptr, Struct3f pos)
{
	using namespace Geometry;
	ptr->m_WorldLevel = (World::WorldHeight)((int)ptr->m_WorldLevel + 1);
	ptr->m_YPos = (((float)ptr->m_WorldLevel / sqrt(2)) * World::TILE_SIZE);
	Struct3f newPos = { ptr->m_XPos, ptr->m_YPos, ptr->m_ZPos };
	Position<SGRender::NTVertex>(&ptr->m_Sprite, pos, newPos, Shape::QUAD);
}

static void DecrLevel(Ov_Sprite::Sprite* ptr, Struct3f pos)
{
	using namespace Geometry;
	ptr->m_WorldLevel = (World::WorldHeight)((int)ptr->m_WorldLevel - 1);
	ptr->m_YPos = (((float)ptr->m_WorldLevel / sqrt(2)) * World::TILE_SIZE);
	Struct3f newPos = { ptr->m_XPos, ptr->m_YPos, ptr->m_ZPos };
	Position<SGRender::NTVertex>(&ptr->m_Sprite, pos, newPos, Shape::QUAD);
}

bool PokemonFollow::checkPerm(World::Direction direction, Struct3f pos)
{
	World::MovementPermissions* perm = World::GetTilePermission(m_Pointer->m_CurrentLevel, World::NextTileInDirection(direction, m_Pointer->m_Tile), m_Pointer->m_WorldLevel);
	switch (*perm)
	{
	case World::MovementPermissions::WALL:
		return false;
	case World::MovementPermissions::STAIRS_NORTH:
		if (direction == World::Direction::NORTH)
		{
			IncLevel(m_Pointer, pos);
		}
		else if (direction == World::Direction::SOUTH)
		{
			DecrLevel(m_Pointer, pos);
		}
		return true;
	case World::MovementPermissions::STAIRS_SOUTH:
		if (direction == World::Direction::SOUTH)
		{
			IncLevel(m_Pointer, pos);
		}
		else if (direction == World::Direction::NORTH)
		{
			DecrLevel(m_Pointer, pos);
		}
		return true;
	case World::MovementPermissions::STAIRS_EAST:
		if (direction == World::Direction::EAST)
		{
			IncLevel(m_Pointer, pos);
		}
		else if (direction == World::Direction::WEST)
		{
			DecrLevel(m_Pointer, pos);
		}
		return true;
	case World::MovementPermissions::STAIRS_WEST:
		if (direction == World::Direction::WEST)
		{
			IncLevel(m_Pointer, pos);
		}
		else if (direction == World::Direction::EAST)
		{
			DecrLevel(m_Pointer, pos);
		}
		return true;
	default:
		return true;
	}
	return true;
}

void PokemonAnimation::update(double deltaTime)
{
	if (m_Pkm->m_Direction != m_LastDirection)
	{
		//(East is default)
		switch (m_Pkm->m_Direction)
		{
		case World::Direction::WEST:
			m_Pkm->m_AnimationOffsetX = 1;
			m_Pkm->m_AnimationOffsetY = 2;
			m_BaseY = 2;
			break;
		case World::Direction::SOUTH:
			m_Pkm->m_AnimationOffsetX = 0;
			m_Pkm->m_AnimationOffsetY = 0;
			m_BaseY = 0;
			break;
		case World::Direction::NORTH:
			m_Pkm->m_AnimationOffsetX = 0;
			m_Pkm->m_AnimationOffsetY = 2;
			m_BaseY = 2;
			break;
		default:
			m_Pkm->m_AnimationOffsetX = 1;
			m_Pkm->m_AnimationOffsetY = 0;
			m_BaseY = 0;
			break;
		}

		m_LastDirection = m_Pkm->m_Direction;
	}

	if (m_Pkm->m_Walking)
	{
		if (m_Timer >= World::WALK_DURATION / 2.05f)
		{
			m_Frame = !m_Frame;
			m_Pkm->m_AnimationOffsetY = m_BaseY + m_Frame;
			m_Timer = 0.0;
		}
		m_Timer += deltaTime;
		return;
	}
	else if (m_Pkm->m_Running)
	{
		if (m_Timer >= World::RUN_DURATION / 4.0f)
		{
			m_Frame = !m_Frame;
			m_Pkm->m_AnimationOffsetY = m_BaseY + m_Frame;
			m_Timer = 0.0;
		}
		m_Timer += deltaTime;
		return;
		return;
	}
}