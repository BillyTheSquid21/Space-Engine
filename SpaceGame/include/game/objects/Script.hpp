#pragma once
#ifndef GAME_SCRIPT_H
#define GAME_SCRIPT_H

#include <stdint.h>
#include <string>
#include <memory>
#include <functional>
#include "game/objects/OverworldSprite.h"

//Debug Struct for use in creating flags/scripts, which then becomes bool array
struct FlagInfo
{
	bool active;
	uint16_t location;
	std::string flagDescription;
};

//Instructions
enum class ScriptInstruction
{
	//Core
	NO_OP, SET_FLAG, JMP, JMP_IF, WAIT_SEC, LOCK_PLAYER, CORE_MAX, //max to give return term
	
	//Non Core
	OPEN_MSG_BOX, CLOSE_MSG_BOX, MSG, CLEAR_TEXT, GIVE_ITEM, TAKE_ITEM,
	FREEZE_OBJECT, UNFREEZE_OBJECT, CGE_DIRECTION, WALK_IN_DIR, RUN_IN_DIR, 
	SET_BUSY,
};

//Message info - using std ints to guarantee bytes
struct MSG_INFO
{
	//Which list of text to take from
	uint32_t languageID : 8;
	uint32_t listID : 8;
	//Where in the list the text instance is
	uint32_t textEntry : 16;
};

struct FLAG_INFO
{
	uint32_t flagLoc : 16;
	uint32_t state : 8;
	uint32_t unused : 8;
};

//Jump info
struct JMP_INFO
{
	uint32_t line : 16;
	uint32_t unused : 16;
};

//If info
struct JMP_IF_INFO
{
	uint32_t flagLoc : 16;
	uint32_t line : 16;
};

//Item info
struct ITEM_INFO
{
	uint32_t itemID : 16;
	uint32_t quantity : 16;
};

//Direction info
struct DIRECTION_INFO
{
	uint32_t direction : 8;	//as direction is 1 byte can just assign the byte
	uint32_t unused : 24;
};

//Busy info
struct BUSY_INFO
{
	uint32_t state : 8;
	uint32_t unused : 24;
};

//Lock player info
struct LOCK_PLAYER_INFO
{
	uint32_t state : 8;
	uint32_t unused : 24;
};

//Information on what to do with instruction
struct InstructionInfo
{
	//contains different info depending on context - all infos take up 4 contiguous bytes and are laid out differently
	union
	{
		MSG_INFO msgInfo;
		ITEM_INFO itemInfo;
		JMP_INFO jmpInfo;
		JMP_IF_INFO jmpIfInfo;
		FLAG_INFO flgInfo;
		DIRECTION_INFO dirInfo;
		BUSY_INFO busyInfo;
		LOCK_PLAYER_INFO lockPlayInfo;
		uint32_t clear;
	};
};

struct ScriptElement
{
	ScriptInstruction instruction;
	InstructionInfo info;
};

typedef std::shared_ptr<ScriptElement[]> Script;
typedef std::shared_ptr<bool[]> FlagArray;

//Script component to be applied to object
class OverworldScript : public UpdateComponent
{
public:
	OverworldScript(Script script, uint16_t size, OvSpr_RunningSprite* player) { m_Script = script; m_Size = size; m_Player = player; }
	void update(double deltaTime) 
	{
		process(m_Index, deltaTime);

		if (m_Index >= m_Size)
		{
			m_Index = 0;
		}
	}

	virtual ScriptElement process(uint16_t index, double deltaTime)
	{
		//Carries out core functionality here that all implementations use
		//Then returns the element to derived process
		ScriptElement el = m_Script[m_Index];
		switch (el.instruction)
		{
		case ScriptInstruction::JMP:
			m_Index = el.info.jmpInfo.line;
			return el;
		case ScriptInstruction::JMP_IF:
			if (m_FlagArray[el.info.jmpIfInfo.flagLoc])
			{
				m_Index = el.info.jmpIfInfo.line;
			}
			return el;
		case ScriptInstruction::SET_FLAG:
			m_FlagArray[el.info.flgInfo.flagLoc] = el.info.flgInfo.state;
			m_Index++;
			return el;
		case ScriptInstruction::WAIT_SEC:
			if (m_Timer >= 1.0f)
			{
				m_Index++;
				m_Timer = 0.0f;
			}
			m_Timer += deltaTime;
			return el;
		case ScriptInstruction::LOCK_PLAYER:
			m_Player->m_Busy = el.info.lockPlayInfo.state;
			m_Index++;
			return el;
		}
		return el;
	}
protected:
	Script m_Script;
	FlagArray m_FlagArray;
	uint16_t m_Index = 0;
	uint16_t m_Size;
	OvSpr_RunningSprite* m_Player;
private:
	float m_Timer = 0.0;
};

#endif
