#pragma once
#ifndef OVERWORLD_SCRIPT_H
#define OVERWORLD_SCRIPT_H

#include <vector>
#include "game/objects/OverworldSprite.h"
#include "game/objects/Script.hpp"
#include "game/gui/GUI.h"
#include "game/utility/GameText.h"

#include "game/utility/Input.hpp"

//Script component to be applied to object
inline static bool WaitScript(float& timer, float endTime, double deltaTime)
{
	if (timer >= endTime)
	{
		timer = 0.0f;
		return true;
	}
	timer += deltaTime;
	return false;
}

inline static bool WaitInput(GameInput* input)
{
	if (input->HELD_E || input->PRESSED_E || input->PRESSED_X)
	{
		return true;
	}
	return false;
}

class OverworldScript : public UpdateComponent
{
public:
	OverworldScript() = default;
	OverworldScript(Script script, uint16_t size, std::shared_ptr<OvSpr_RunningSprite> player, FlagArray* flags, GameInput* input) { m_Script = script; m_Size = size; m_Player = player; m_FlagArray = flags; m_Input = input; }
	void linkText(GameGUI::TextBoxBuffer* buffer) { m_TextBuffer = buffer; }
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
			if (WaitScript(m_Timer, 1.0f, deltaTime))
			{
				m_Index++;
			}
			return true;
		case ScriptInstruction::PLAYER_LOCK:
			m_Player->m_Busy = el.info.boolInfo.state;
			m_Index++;
			return true;
		case ScriptInstruction::PLAYER_FACE:
			m_Player->m_Direction = (World::Direction)el.info.dirInfo.direction;
			m_Index++;
			return true;
		case ScriptInstruction::PLAYER_WALK:
			if (!m_Player->m_Walking)
			{
				m_Player->m_Walking = true;
				m_Player->m_Controlled = true;
				World::ModifyTilePerm(m_Player->m_CurrentLevel, m_Player->m_Direction, m_Player->m_Tile, m_Player->m_WorldLevel, m_Player->m_LastPermission, m_Player->m_LastPermissionPtr);
			}
			else if (m_Player->m_Timer >= World::WALK_DURATION)
			{
				m_Player->m_Walking = false;
				m_Player->m_Controlled = false;
				m_Player->m_Timer = 0.0;
				Ov_Translation::CentreOnTileSprite(m_Player);
				m_Index++;
				return true;
			}
			Ov_Translation::WalkSprite(m_Player, deltaTime);
			return true;
		case ScriptInstruction::PLAYER_RUN:
			if (!m_Player->m_Running)
			{
				m_Player->m_Running = true;
				m_Player->m_Controlled = true;
				World::ModifyTilePerm(m_Player->m_CurrentLevel, m_Player->m_Direction, m_Player->m_Tile, m_Player->m_WorldLevel, m_Player->m_LastPermission, m_Player->m_LastPermissionPtr);
			}
			else if (m_Player->m_Timer >= World::RUN_DURATION)
			{
				m_Player->m_Running = false;
				m_Player->m_Controlled = false;
				m_Player->m_Timer = 0.0;
				Ov_Translation::CentreOnTileSprite(m_Player);
				m_Index++;
				return true;
			}
			Ov_Translation::RunSprite(m_Player, deltaTime);
			return true;
		case ScriptInstruction::OPEN_MSG_BOX:
			m_TextBuffer->showTextBox = true;
			m_Index++;
			return true;
		case ScriptInstruction::CLOSE_MSG_BOX:
			m_TextBuffer->showTextBox = false;
			m_Index++;
			return true;
		case ScriptInstruction::MSG:
			if (!m_Messaging)
			{
				m_TextBuffer->buffer.pushBuffer(GameText::FindMessage(el.info.msgInfo));
				m_TextBuffer->buffer.nextPage();
				m_Messaging = true;
				return true;
			}
			else if (!m_TextBuffer->buffer.isReady())
			{
				if (!WaitScript(m_Timer, 0.4f, deltaTime))
				{
					return true;
				}
				if (WaitInput(m_Input) && m_TextBuffer->buffer.isNextPageReady())
				{
					m_TextBuffer->buffer.nextPage();
					m_Timer = 0.0f;
					return true;
				}
				return true;
			}
			if (!WaitScript(m_Timer, 0.4f, deltaTime))
			{
				return true;
			}
			m_Messaging = false;
			m_Index++;
			return true;
		case ScriptInstruction::CLEAR_TEXT:
			if (!m_TextBuffer->buffer.isNextPageReady())
			{
				return true;
			}
			m_TextBuffer->buffer.line1 = "";
			m_TextBuffer->buffer.line2 = "";
			m_Index++;
			return true;
		case ScriptInstruction::WAIT_INPUT:
			if (WaitInput(m_Input))
			{
				m_Index++;
			}
			return true;
		}
		return false;
	}
public:
	Script m_Script;
	FlagArray* m_FlagArray;
	uint16_t m_Index = 0;
	uint16_t m_Size;
	std::shared_ptr<OvSpr_RunningSprite> m_Player;
	GameInput* m_Input = nullptr;
	GameGUI::TextBoxBuffer* m_TextBuffer = nullptr;
	bool m_Messaging = false;
private:
	float m_Timer = 0.0;
};

#endif