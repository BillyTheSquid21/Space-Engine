#pragma once
#ifndef NPC_AI_H
#define NPC_AI_H

#include <vector>
#include "game/objects/OverworldScript.hpp"
#include "game/objects/ScriptParse.h"
#include "game/GUI/GUI.h"

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
		case ScriptInstruction::CGE_DIR:
			m_NPC->m_Direction = (World::Direction)el.info.dirInfo.direction;
			m_Index++;
			return true;
		case ScriptInstruction::WALK_IN_DIR:
			if (!m_NPC->m_Walking)
			{
				m_NPC->m_Walking = true;
				World::ModifyTilePerm(m_NPC->m_CurrentLevel, m_NPC->m_Direction, m_NPC->m_Tile, m_NPC->m_WorldLevel);
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
		case ScriptInstruction::RUN_IN_DIR:
			if (!m_NPC->m_Running)
			{
				m_NPC->m_Running = true;
				World::ModifyTilePerm(m_NPC->m_CurrentLevel, m_NPC->m_Direction, m_NPC->m_Tile, m_NPC->m_WorldLevel);
			}
			else if (m_NPC->m_Timer >= World::RUN_DURATION)
			{
				m_NPC->m_Running = false;
				m_NPC->m_Timer = 0.0;
				Ov_Translation::CentreOnTileSprite(m_NPC);
				m_Index++;
				return true;
			}
			Ov_Translation::RunSprite(m_NPC, deltaTime);
			return true;
		case ScriptInstruction::FREEZE_OBJECT:
			m_NPC->m_Busy = (bool)el.info.lockInfo.state;
			m_Index++;
			return true;
		case ScriptInstruction::FACE_PLAYER:
			m_NPC->m_Direction = World::OppositeDirection(m_Player->m_Direction);
			m_Index++;
			return true;
		case ScriptInstruction::WAIT_INTERACT:
			if (m_Input->HELD_E && !m_NPC->m_Busy)
			{
				if (World::CheckPlayerInteracting(m_Player->m_Tile, m_NPC->m_Tile, m_Player->m_Direction))
				{
					m_Index++;
				}
			}
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
	NPC_RandWalk(World::Direction* direct, World::WorldHeight* level, float* y, bool* busy, bool* walking, Norm_Tex_Quad* sprite, double* walkTimer) {	m_Direction = direct; m_WorldLevel = level; m_YPos = y; m_Busy = busy; m_Walking = walking; m_Sprite = sprite; m_WalkTimer = walkTimer;
	if (!s_Random.isSeeded()) { s_Random.seed(0.0f, MAX_SEED); } m_CoolDownTimer = s_Random.next() / 8.0f;};
	
	void linkLocation(unsigned int* tileX, unsigned int* tileZ, float* x, float* z, World::LevelID* level) { m_TileX = tileX; m_TileZ = tileZ; m_XPos = x; m_ZPos = z; m_CurrentLevel = level; };

	void update(double deltaTime);
	bool* m_Busy = nullptr;

private:
	//Busy status is set by other components that carry out instructions - while busy the next instuction wont be read
	//AI class must be updated first per frame
	World::Direction* m_Direction = nullptr; World::WorldHeight* m_WorldLevel = nullptr;
	float* m_YPos = nullptr;

	//Applies to either walking or running, as npcs will 
	bool* m_Walking = nullptr;
	Norm_Tex_Quad* m_Sprite = nullptr;
	float m_CoolDownTimer = 0.0f;
	double* m_WalkTimer = nullptr;

	//Moving helpers
	bool canWalk();
	void startWalk();
	void cycleEnd();
	void randomWalk();

	//Static randomness for randwalk class
	static RandomContainer s_Random;
	static const float MAX_SEED;
};

//Script for NPC's with NPC specific functionality
template<typename SpriteType>
class NPC_Script : public OverworldScript
{
public:
	//Setters
	using OverworldScript::OverworldScript;
	void linkNPC(std::shared_ptr<SpriteType> npc) { m_NPCData = npc; }
	void linkUpdateMessage(std::function<void(Message message)> msg) { m_MessageUpdate = msg; }

	ScriptElement process(uint16_t index, double deltaTime)
	{
		ScriptElement el = OverworldScript::process(index, deltaTime);
		//If instruction is core, return
		if ((int)el.instruction <= (int)ScriptInstruction::CORE_MAX)
		{
			return el;
		}
		//Process
		switch (el.instruction)
		{
		case ScriptInstruction::SET_BUSY:
			m_NPCData->m_Busy = el.info.busyInfo.state;
			m_Index++;
			break;
		case ScriptInstruction::CGE_DIR:
			m_NPCData->m_Direction = (World::Direction)el.info.dirInfo.direction;
			m_Index++;
			break;
		case ScriptInstruction::WALK_IN_DIR:
			if (!m_NPCData->m_Walking)
			{
				m_NPCData->m_Walking = true;
				World::ModifyTilePerm(m_NPCData->m_CurrentLevel, m_NPCData->m_Direction, { m_NPCData->m_TileX, m_NPCData->m_TileZ });
			}
			else if (m_NPCData->m_Timer >= World::WALK_DURATION)
			{
				m_NPCData->m_Walking = false;
				m_NPCData->m_Timer = 0.0;
				Ov_Translation::CentreOnTileSprite(m_NPCData);
				m_Index++;
				break;
			}
			Ov_Translation::WalkSprite(m_NPCData, deltaTime);
			break;
		case ScriptInstruction::RUN_IN_DIR:
			if (!m_NPCData->m_Running)
			{
				m_NPCData->m_Running = true;
				World::ModifyTilePerm(m_NPCData->m_CurrentLevel, m_NPCData->m_Direction, { m_NPCData->m_TileX, m_NPCData->m_TileZ });
			}
			else if (m_NPCData->m_Timer >= World::RUN_DURATION)
			{
				m_NPCData->m_Running = false;
				m_NPCData->m_Timer = 0.0;
				Ov_Translation::CentreOnTileSprite(m_NPCData);
				m_Index++;
				break;
			}
			Ov_Translation::RunSprite(m_NPCData, deltaTime);
			break;
		}
		return el;
	}
private:
	std::function<void(Message message)> m_MessageUpdate;
	std::shared_ptr<SpriteType> m_NPCData; //assumes most data heavy sprite
};

#endif 
