#pragma once
#ifndef NPC_AI_H
#define NPC_AI_H

#include <vector>
#include "game/objects/OverworldSprite.h"
#include "game/objects/Script.hpp"

class NPC_RandWalk : public TilePosition
{
public:
	NPC_RandWalk(World::Direction* direct, bool* busy, bool* walking, TextureQuad* sprite, double* walkTimer) {	m_Direction = direct; m_Busy = busy; m_Walking = walking; m_Sprite = sprite; m_WalkTimer = walkTimer;
	if (!s_Random.isSeeded()) { s_Random.seed(0.0f, MAX_SEED); } m_CoolDownTimer = s_Random.next() / 8.0f;};
	
	void linkLocation(unsigned int* tileX, unsigned int* tileZ, float* x, float* z, World::LevelID* level) { m_TileX = tileX; m_TileZ = tileZ; m_XPos = x; m_ZPos = z; m_CurrentLevel = level; };

	void update(double deltaTime);
	bool* m_Busy = nullptr;

private:
	//Busy status is set by other components that carry out instructions - while busy the next instuction wont be read
	//AI class must be updated first per frame
	World::Direction* m_Direction = nullptr;

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
	void linkText(std::string* t1, std::string* t2, std::function<std::string& (MSG_INFO info)> txt, bool* showBox) { m_TextLine1 = t1; m_TextLine2 = t2; m_TextFinder = txt; m_ShowTextBox = showBox; }
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
		case ScriptInstruction::OPEN_MSG_BOX:
			*m_ShowTextBox = true;
			m_Index++;
			break;
		case ScriptInstruction::CLOSE_MSG_BOX:
			*m_ShowTextBox = false;
			m_Index++;
			break;
		case ScriptInstruction::MSG:
			if (m_OnLine_1)
			{
				*m_TextLine1 = m_TextFinder(el.info.msgInfo);
				m_Index++;
				m_OnLine_1 = false;
				break;
			}
			*m_TextLine2 = m_TextFinder(el.info.msgInfo);
			m_Index++;
			m_OnLine_1 = true;
			break;
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
	bool* m_ShowTextBox = nullptr;
	std::string* m_TextLine1 = nullptr;
	std::string* m_TextLine2 = nullptr;
	bool m_OnLine_1 = true;
	std::function<void(Message message)> m_MessageUpdate;
	std::function<std::string& (MSG_INFO info)> m_TextFinder;
	std::shared_ptr<SpriteType> m_NPCData; //assumes most data heavy sprite
};

#endif 
