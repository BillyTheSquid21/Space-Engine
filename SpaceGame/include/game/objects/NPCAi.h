#pragma once
#ifndef NPC_AI_H
#define NPC_AI_H

#include <vector>
#include "game/objects/OverworldScript.hpp"
#include "game/objects/ScriptParse.h"
#include "game/GUI/GUI.h"
#include "game/utility/Random.hpp"

class NPC_OverworldScript : public OverworldScript
{
public:
	using OverworldScript::OverworldScript;
	void linkNPC(std::shared_ptr<OvSpr_RunningSprite> npc) { m_NPC = npc; };

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
			if (!m_NPC->m_Walking)
			{
				m_NPC->m_Walking = true;
				World::ModifyTilePerm(m_NPC->m_CurrentLevel, m_NPC->m_Direction, m_NPC->m_Tile, m_NPC->m_WorldLevel, m_NPC->m_LastPermission, m_NPC->m_LastPermissionPtr);
			}
			else if (m_NPC->m_Timer >= World::WALK_DURATION)
			{
				m_NPC->m_Walking = false;
				m_NPC->m_Timer = 0.0;
				Ov_Translation::CentreOnTileSprite(m_NPC);
				m_Index++;
				return true;
			}
			Ov_Translation::WalkSprite(m_NPC, deltaTime);
			return true;
		case ScriptInstruction::NPC_RUN:
			if (!m_NPC->m_Running)
			{
				m_NPC->m_Running = true;
				m_NPC->m_Busy = true;
				World::ModifyTilePerm(m_NPC->m_CurrentLevel, m_NPC->m_Direction, m_NPC->m_Tile, m_NPC->m_WorldLevel, m_NPC->m_LastPermission, m_NPC->m_LastPermissionPtr);
			}
			else if (m_NPC->m_Timer >= World::RUN_DURATION)
			{
				m_NPC->m_Running = false;
				m_NPC->m_Busy = false;
				m_NPC->m_Timer = 0.0;
				Ov_Translation::CentreOnTileSprite(m_NPC);
				m_Index++;
				return true;
			}
			Ov_Translation::RunSprite(m_NPC, deltaTime);
			return true;
		case ScriptInstruction::FREEZE_OBJECT:
			m_NPC->m_Busy = (bool)el.info.boolInfo.state;
			m_Index++;
			return true;
		case ScriptInstruction::FACE_PLAYER:
			m_NPC->m_Direction = World::OppositeDirection(m_Player->m_Direction);
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
				Ov_Translation::CentreOnTileSprite(m_Player);
				Ov_Translation::CentreOnTileSprite(m_NPC);
				m_Index++;
				return true;
			}
			Ov_Translation::WalkSprite(m_NPC, deltaTime);
			Ov_Translation::WalkSprite(m_Player, deltaTime);
			return true;
		case ScriptInstruction::SHOW_SPRITE:
			if (el.info.boolInfo.state)
			{
				m_NPC->messageAllRender((uint32_t)Message::ACTIVATE);
			}
			else
			{
				m_NPC->messageAllRender((uint32_t)Message::DEACTIVATE);
			}
			m_Index++;
			return true;
		default:
			return false;
		}
	}

private:
	std::shared_ptr<OvSpr_RunningSprite> m_NPC;
};

//Creates either a heap or stack allocated npc script
std::shared_ptr<NPC_OverworldScript> AllocateNPCOvScript(std::string filePath, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<OvSpr_Sprite> npc, std::shared_ptr<OvSpr_RunningSprite> player, GameInput* input);
NPC_OverworldScript CreateNPCOvScript(std::string filePath, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<OvSpr_Sprite> npc, std::shared_ptr<OvSpr_RunningSprite> player, GameInput* input);


class NPC_RandWalk : public TilePosition
{
public:
	NPC_RandWalk(std::shared_ptr<OvSpr_WalkingSprite> npc) {
		m_NPC = npc; m_CurrentLevel = &m_NPC->m_CurrentLevel; m_XPos = &m_NPC->m_XPos; m_ZPos = &m_NPC->m_ZPos; m_TileX = &m_NPC->m_Tile.x; m_TileZ = &m_NPC->m_Tile.z;
	if (!s_Random.isSeeded()) { s_Random.seed(0.0f, MAX_SEED); } m_CoolDownTimer = s_Random.next() / 8.0f;};

	void update(double deltaTime);

private:
	//Busy status is set by other components that carry out instructions - while busy the next instuction wont be read
	//AI class must be updated first per frame
	std::shared_ptr<OvSpr_WalkingSprite> m_NPC;
	float m_CoolDownTimer = 0.0f;

	//Moving helpers
	bool canWalk();
	void startWalk();
	void cycleEnd();
	void randomWalk();

	//Static randomness for randwalk class
	static RandomContainer s_Random;
	static const float MAX_SEED;
};

#endif 
