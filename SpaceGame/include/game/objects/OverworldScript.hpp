#pragma once
#ifndef OVERWORLD_SCRIPT_H
#define OVERWORLD_SCRIPT_H

#include <vector>
#include "game/objects/OverworldSprite.h"
#include "game/objects/Script.hpp"
#include "game/utility/GameText.h"

#include "game/utility/Input.hpp"

//Script component to be applied to object
class OverworldScript : public UpdateComponent
{
public:
	OverworldScript(Script script, uint16_t size, OvSpr_RunningSprite* player, FlagArray* flags, GameInput* input) { m_Script = script; m_Size = size; m_Player = player; m_FlagArray = flags; m_Input = input; }
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
		case ScriptInstruction::WAIT_INPUT:
			if (m_Input->HELD_E || m_Input->PRESSED_X)
			{
				m_Index++;
			}
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
	GameInput* m_Input = nullptr;
	std::string* m_TextLine1 = nullptr;
	std::string* m_TextLine2 = nullptr;
	bool m_OnLine_1 = true;
private:
	float m_Timer = 0.0;
};

#endif