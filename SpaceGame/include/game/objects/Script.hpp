#pragma once
#ifndef GAME_SCRIPT_H
#define GAME_SCRIPT_H

#include <stdint.h>
#include <string>
#include <memory>
#include <functional>
#include "core/GameObject.hpp"

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
	//Core - current max = 3
	NO_OP, SET_FLAG, JMP, JMP_IF, INSTR_MAX, //max to give return term
	
	//Other
	OPEN_MSG_BOX, CLOSE_MSG_BOX, MSG, GIVE_ITEM, TAKE_ITEM,
	FREEZE_OBJECT, UNFREEZE_OBJECT,
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
	OverworldScript(Script script, uint16_t size) { m_Script = script; m_Size = size; }
	void update(double deltaTime) 
	{
		process(m_Index);

		if (m_Index >= m_Size)
		{
			m_Index = 0;
		}
	}

	virtual ScriptElement process(uint16_t index)
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
		}
		return el;
	}
protected:
	Script m_Script;
	FlagArray m_FlagArray;
	uint16_t m_Index = 0;
	uint16_t m_Size;
};

//Script for NPC's with NPC specific functionality
class NPC_Script : public OverworldScript
{
public:
	//Setters
	using OverworldScript::OverworldScript;
	void setupText(std::string* t1, std::string* t2, std::function<std::string& (MSG_INFO info)> txt, bool* showBox) { m_TextLine1 = t1; m_TextLine2 = t2; m_TextFinder = txt; m_ShowTextBox = showBox; }

	ScriptElement process(uint16_t index)
	{
		ScriptElement el = OverworldScript::process(index);
		//If instruction is core, return
		if ((int)el.instruction <= (int)ScriptInstruction::INSTR_MAX)
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
			EngineLog(m_TextFinder(el.info.msgInfo));

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
		}
		return el;
	}
private:
	bool* m_ShowTextBox = nullptr;
	std::string* m_TextLine1 = nullptr;
	std::string* m_TextLine2 = nullptr;
	bool m_OnLine_1 = true;
	std::function<void(Message message, unsigned int id)> m_MessageUpdate;
	std::function<std::string&(MSG_INFO info)> m_TextFinder;
};

#endif
