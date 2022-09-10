#pragma once
#ifndef NPC_AI_H
#define NPC_AI_H

#include <vector>
#include "game/objects/OverworldScript.h"
#include "game/objects/ScriptParse.h"
#include "game/GUI/GUI.h"
#include "utility/Random.h"
#include "game/data/PlayerData.hpp"

class NPC_OverworldScript : public OverworldScript
{
public:
	using OverworldScript::OverworldScript;
	void linkNPC(std::shared_ptr<Ov_Sprite::RunSprite> npc) { m_NPC = npc; };

	virtual bool process(double deltaTime)
	{
		if (OverworldScript::process(deltaTime))
		{
			return true;
		}
		ScriptElement el = m_Script[m_Index];
		switch (el.instruction)
		{
		case ScriptInstruction::NPC_FACE:
			m_NPC->m_Direction = (World::Direction)el.info.dirInfo.direction;
			m_Index++;
			return true;
		case ScriptInstruction::NPC_WALK:
			if (SpriteWalk(m_NPC.get(), m_NPC->m_Busy, deltaTime))
			{
				m_Index++;
			}
			return true;
		case ScriptInstruction::NPC_RUN:
			if (SpriteRun(m_NPC.get(), m_NPC->m_Busy, deltaTime))
			{
				m_Index++;
			}
			return true;
		case ScriptInstruction::NPC_WALK_TO_TILE:
			if (m_Path.directionsIndex == -1)
			{
				m_Path = PathFinding::GetPath(m_NPC.get(), { (int)el.info.tileInfo.x, (int)el.info.tileInfo.z }, (World::WorldHeight)el.info.tileInfo.h);
				if (m_Path.directionsIndex == -2)
				{
					m_Index++;
					return true;
				}
				m_NPC->m_Direction = m_Path.directions[m_Path.directionsIndex];
				m_Path.directionsIndex++;
			}
			else if (m_Path.directionsIndex >= m_Path.directions.size())
			{
				if (SpriteWalk(m_NPC.get(), m_NPC->m_Busy, deltaTime))
				{
					PathFinding::Path path; m_Path = path;
					m_Index++;
				}
				return true;
			}
			else if (SpriteWalk(m_NPC.get(), m_NPC->m_Busy, deltaTime))
			{
				PathFinding::ValidatePath(m_NPC.get(), { (int)el.info.tileInfo.x, (int)el.info.tileInfo.z }, (World::WorldHeight)el.info.tileInfo.h, m_Path);
				m_NPC->m_Direction = m_Path.directions[m_Path.directionsIndex];
				m_Path.directionsIndex++;
			}
			return true;
		case ScriptInstruction::NPC_RUN_TO_TILE:
			if (m_Path.directionsIndex == -1)
			{
				m_Path = PathFinding::GetPath(m_NPC.get(), { (int)el.info.tileInfo.x, (int)el.info.tileInfo.z }, (World::WorldHeight)el.info.tileInfo.h);
				if (m_Path.directionsIndex == -2)
				{
					m_Index++;
					return true;
				}
				m_NPC->m_Direction = m_Path.directions[m_Path.directionsIndex];
				m_Path.directionsIndex++;
			}
			else if (m_Path.directionsIndex >= m_Path.directions.size())
			{
				if (SpriteRun(m_NPC.get(), m_NPC->m_Busy, deltaTime))
				{
					PathFinding::Path path; m_Path = path;
					m_Index++;
				}
				return true;
			}
			else if (SpriteRun(m_NPC.get(), m_NPC->m_Busy, deltaTime))
			{
				PathFinding::ValidatePath(m_NPC.get(), { (int)el.info.tileInfo.x, (int)el.info.tileInfo.z }, (World::WorldHeight)el.info.tileInfo.h, m_Path);
				m_NPC->m_Direction = m_Path.directions[m_Path.directionsIndex];
				m_Path.directionsIndex++;
			}
			return true;
		case ScriptInstruction::FREEZE_OBJECT:
			m_NPC->m_Busy = (bool)el.info.boolInfo.state;
			m_Index++;
			return true;
		case ScriptInstruction::FACE_PLAYER:
			m_NPC->m_Direction = World::OppositeDirection(m_Player->m_Direction);
			m_Index++;
			return true;
		case ScriptInstruction::FACE_WITH_PLAYER:
			m_NPC->m_Direction = m_Player->m_Direction;
			m_Index++;
			return true;
		case ScriptInstruction::WAIT_INTERACT:
			if (m_Input->HELD_E && !m_NPC->m_Busy && !m_Player->m_Busy)
			{
				if (World::CheckPlayerInteracting(m_Player->m_Tile, m_NPC->m_Tile, m_Player->m_Direction))
				{
					m_Index++;
				}
			}
			return true;
		case ScriptInstruction::NPC_AND_PLAYER_WALK:
			if (!m_NPC->m_Walking)
			{
				m_NPC->m_Walking = true;
				World::ModifyTilePerm(m_NPC->m_CurrentLevel, m_NPC->m_Direction, m_NPC->m_Tile, m_NPC->m_WorldLevel, m_NPC->m_LastPermission, m_NPC->m_LastPermissionPtr);
			}
			if (!m_Player->m_Walking)
			{
				m_Player->m_Walking = true;
				m_Player->m_Controlled = true;
				World::ModifyTilePerm(m_Player->m_CurrentLevel, m_Player->m_Direction, m_Player->m_Tile, m_Player->m_WorldLevel, m_NPC->m_LastPermission, m_NPC->m_LastPermissionPtr);
			}
			if ((m_NPC->m_Walking && m_Player->m_Walking) && (m_Player->m_Timer >= World::WALK_DURATION && m_NPC->m_Timer >= World::WALK_DURATION))
			{
				m_Player->m_Walking = false; m_NPC->m_Walking = false;
				m_Player->m_Controlled = false;
				m_Player->m_Timer = 0.0; m_NPC->m_Timer = 0.0;
				Ov_Translation::CentreOnTile(m_Player->m_CurrentLevel, m_Player->m_WorldLevel, &m_Player->m_XPos, &m_Player->m_YPos, &m_Player->m_ZPos, m_Player->m_Tile, &m_Player->m_Sprite, &m_Player->m_CurrentIsSlope);
				Ov_Translation::CentreOnTile(m_NPC->m_CurrentLevel, m_NPC->m_WorldLevel, &m_NPC->m_XPos, &m_NPC->m_YPos, &m_NPC->m_ZPos, m_NPC->m_Tile, &m_NPC->m_Sprite, &m_NPC->m_CurrentIsSlope);
				m_Index++;
				return true;
			}
			Ov_Translation::Walk(&m_NPC->m_Direction, &m_NPC->m_XPos, &m_NPC->m_ZPos, &m_NPC->m_Sprite, deltaTime, &m_NPC->m_Timer);
			Ov_Translation::Walk(&m_Player->m_Direction, &m_Player->m_XPos, &m_Player->m_ZPos, &m_Player->m_Sprite, deltaTime, &m_Player->m_Timer);
			return true;
		case ScriptInstruction::SHOW_SPRITE:
			if (el.info.boolInfo.state)
			{
				m_NPC->messageAllRender((uint32_t)SGObject::Message::ACTIVATE);
			}
			else
			{
				m_NPC->messageAllRender((uint32_t)SGObject::Message::DEACTIVATE);
			}
			m_Index++;
			return true;
		default:
			return false;
		}
	}

private:
	std::shared_ptr<Ov_Sprite::RunSprite> m_NPC;
};

//Creates either a heap or stack allocated npc script
std::shared_ptr<NPC_OverworldScript> AllocateNPCOvScript(std::string filePath, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<Ov_Sprite::Sprite> npc, std::shared_ptr<Ov_Sprite::RunSprite> player);
NPC_OverworldScript CreateNPCOvScript(std::string filePath, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<Ov_Sprite::Sprite> npc, std::shared_ptr<Ov_Sprite::RunSprite> player);


class NPC_RandWalk : public Ov_Sprite::TilePosition
{
public:
	NPC_RandWalk(std::shared_ptr<Ov_Sprite::WalkSprite> npc) {
		m_NPC = npc; m_CurrentLevel = &m_NPC->m_CurrentLevel; m_XPos = &m_NPC->m_XPos; m_ZPos = &m_NPC->m_ZPos; m_Tile = &m_NPC->m_Tile;
	if (!s_Random.isSeeded()) { s_Random.seed(0.0f, MAX_SEED); } m_CoolDownTimer = s_Random.next() / 8.0f;};

	void update(double deltaTime);

private:
	//Busy status is set by other components that carry out instructions - while busy the next instuction wont be read
	//AI class must be updated first per frame
	std::shared_ptr<Ov_Sprite::WalkSprite> m_NPC;
	float m_CoolDownTimer = 0.0f;

	//Moving helpers
	void startWalk();
	void cycleEnd();
	void randomWalk();

	//Static randomness for randwalk class
	static SGRandom::RandomContainer s_Random;
	static const float MAX_SEED;
};

#endif 
