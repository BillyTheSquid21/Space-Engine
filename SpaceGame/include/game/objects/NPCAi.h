#pragma once
#ifndef NPC_AI_H
#define NPC_AI_H

#include <vector>
#include "game/objects/OverworldSprite.h"
#include "game/objects/Script.hpp"
#include "game/utility/GameText.h"

//Script component to be applied to object
class OverworldScript : public UpdateComponent
{
public:
	OverworldScript(Script script, uint16_t size, OvSpr_RunningSprite* player, FlagArray* flags) { m_Script = script; m_Size = size; m_Player = player; m_FlagArray = flags; }
	void linkText(std::string* t1, std::string* t2, bool* showBox) { m_TextLine1 = t1; m_TextLine2 = t2; m_ShowTextBox = showBox; }
	void update(double deltaTime)
	{
		process(deltaTime);

		if (m_Index >= m_Size)
		{
			m_Index = 0;
		}
	}

	virtual bool process(double deltaTime)
	{
		//Carries out core functionality here that all implementations use
		//Then returns whether index is fully processed
		ScriptElement el = m_Script[m_Index];
		switch (el.instruction)
		{
		case ScriptInstruction::JMP:
			m_Index = el.info.jmpInfo.line;
			return true;
		case ScriptInstruction::JMP_IF:
			if (m_FlagArray->at(el.info.jmpIfInfo.flagLoc))
			{
				m_Index = el.info.jmpIfInfo.line;
				return true;
			}
			m_Index++;
			return true;
		case ScriptInstruction::SET_FLAG:
			m_FlagArray->at(el.info.flgInfo.flagLoc) = el.info.flgInfo.state;
			m_Index++;
			return true;
		case ScriptInstruction::WAIT_SEC:
			if (m_Timer >= 1.0f)
			{
				m_Index++;
				m_Timer = 0.0f;
			}
			m_Timer += deltaTime;
			return true;
		case ScriptInstruction::LOCK_PLAYER:
			m_Player->m_Busy = el.info.lockInfo.state;
			m_Index++;
			return true;
		case ScriptInstruction::OPEN_MSG_BOX:
			*m_ShowTextBox = true;
			m_Index++;
			return true;
		case ScriptInstruction::CLOSE_MSG_BOX:
			*m_ShowTextBox = false;
			m_Index++;
			return true;
		case ScriptInstruction::MSG:
			if (m_OnLine_1)
			{
				*m_TextLine1 = GameText::FindMessage(el.info.msgInfo);
				m_Index++;
				m_OnLine_1 = false;
				return true;
			}
			*m_TextLine2 = GameText::FindMessage(el.info.msgInfo);
			m_Index++;
			m_OnLine_1 = true;
			return true;
		case ScriptInstruction::CLEAR_TEXT:
			*m_TextLine1 = "";
			*m_TextLine2 = "";
			m_OnLine_1 = true;
			m_Index++;
			return true;
		}
		return false;
	}
protected:
	Script m_Script;
	FlagArray* m_FlagArray;
	uint16_t m_Index = 0;
	uint16_t m_Size;
	OvSpr_RunningSprite* m_Player;
	bool* m_ShowTextBox = nullptr;
	std::string* m_TextLine1 = nullptr;
	std::string* m_TextLine2 = nullptr;
	bool m_OnLine_1 = true;
private:
	float m_Timer = 0.0;
};

class NPC_OverworldScript : public OverworldScript
{
public:
	using OverworldScript::OverworldScript;
	void linkNPC(OvSpr_RunningSprite* npc) { m_NPC = npc; };

	virtual bool process(double deltaTime)
	{
		if (OverworldScript::process(deltaTime))
		{
			return true;
		}
		ScriptElement el = m_Script[m_Index];
		switch (el.instruction)
		{
		case ScriptInstruction::CGE_DIRECTION:
			m_NPC->m_Direction = (World::Direction)el.info.dirInfo.direction;
			m_Index++;
			return true;
		case ScriptInstruction::WALK_IN_DIR:
			if (!m_NPC->m_Walking)
			{
				m_NPC->m_Walking = true;
				World::ModifyTilePerm(m_NPC->m_CurrentLevel, m_NPC->m_Direction, { m_NPC->m_TileX, m_NPC->m_TileZ });
			}
			else if (m_NPC->m_Timer >= World::WALK_DURATION)
			{
				m_NPC->m_Walking = false;
				m_NPC->m_Timer = 0.0;
				Ov_Translation::CentreOnTile(m_NPC->m_CurrentLevel, m_NPC->m_WorldLevel, &m_NPC->m_XPos, &m_NPC->m_YPos, &m_NPC->m_ZPos, m_NPC->m_TileX, m_NPC->m_TileZ, &m_NPC->m_Sprite);
				m_Index++;
				return true;
			}
			Ov_Translation::Walk(&m_NPC->m_Direction, &m_NPC->m_XPos, &m_NPC->m_ZPos, &m_NPC->m_Sprite, deltaTime, &m_NPC->m_Timer);
			return true;
		case ScriptInstruction::RUN_IN_DIR:
			if (!m_NPC->m_Running)
			{
				m_NPC->m_Running = true;
				World::ModifyTilePerm(m_NPC->m_CurrentLevel, m_NPC->m_Direction, { m_NPC->m_TileX, m_NPC->m_TileZ });
			}
			else if (m_NPC->m_Timer >= World::RUN_DURATION)
			{
				m_NPC->m_Running = false;
				m_NPC->m_Timer = 0.0;
				Ov_Translation::CentreOnTile(m_NPC->m_CurrentLevel, m_NPC->m_WorldLevel, &m_NPC->m_XPos, &m_NPC->m_YPos, &m_NPC->m_ZPos, m_NPC->m_TileX, m_NPC->m_TileZ, &m_NPC->m_Sprite);
				m_Index++;
				return true;
			}
			Ov_Translation::Run(&m_NPC->m_Direction, &m_NPC->m_XPos, &m_NPC->m_ZPos, &m_NPC->m_Sprite, deltaTime, &m_NPC->m_Timer);
			return true;
		case ScriptInstruction::FREEZE_OBJECT:
			m_NPC->m_Busy = (bool)el.info.lockInfo.state;
			m_Index++;
			return true;
		}
	}

private:
	OvSpr_RunningSprite* m_NPC;
};

class NPC_RandWalk : public TilePosition
{
public:
	NPC_RandWalk(World::Direction* direct, World::WorldLevel* level, float* y, bool* busy, bool* walking, TextureQuad* sprite, double* walkTimer) {	m_Direction = direct; m_WorldLevel = level; m_YPos = y; m_Busy = busy; m_Walking = walking; m_Sprite = sprite; m_WalkTimer = walkTimer;
	if (!s_Random.isSeeded()) { s_Random.seed(0.0f, MAX_SEED); } m_CoolDownTimer = s_Random.next() / 8.0f;};
	
	void linkLocation(unsigned int* tileX, unsigned int* tileZ, float* x, float* z, World::LevelID* level) { m_TileX = tileX; m_TileZ = tileZ; m_XPos = x; m_ZPos = z; m_CurrentLevel = level; };

	void update(double deltaTime);
	bool* m_Busy = nullptr;

private:
	//Busy status is set by other components that carry out instructions - while busy the next instuction wont be read
	//AI class must be updated first per frame
	World::Direction* m_Direction = nullptr; World::WorldLevel* m_WorldLevel = nullptr;
	float* m_YPos = nullptr;

	//Applies to either walking or running, as npcs will 
	bool* m_Walking = nullptr;
	TextureQuad* m_Sprite = nullptr;
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
		case ScriptInstruction::CGE_DIRECTION:
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
