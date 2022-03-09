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
	NO_OP, OPEN_MSG_BOX, CLOSE_MSG_BOX, MSG, GIVE_ITEM, TAKE_ITEM, 
	SET_FLAG, JMP, JMP_IF, FREEZE_OBJECT, UNFREEZE_OBJECT,
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

//Item info
struct ITEM_INFO
{
	uint32_t itemID : 16;
	uint32_t quantity : 16;
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

//Information on what to do with instruction
struct InstructionInfo
{
	//contains different info depending on context - all infos take up 4 contiguous bytes and are laid out differently
	union
	{
		MSG_INFO msgInfo;
		ITEM_INFO itemInfo;
		JMP_INFO jmpInfo;
		uint32_t clear;
	};
};

struct ScriptElement
{
	ScriptInstruction instruction;
	InstructionInfo info;
};

typedef std::shared_ptr<ScriptElement[]> Script;

//Script component to be applied to object
class OverworldScript : public UpdateComponent
{
public:
	OverworldScript(Script script, uint16_t size, std::function <void(Message message, unsigned int id)> updmsg) { m_Script = script; m_Size = size; m_MessageUpdate = updmsg; }
	void update(double deltaTime) 
	{
		process(m_Index);

		if (m_Index >= m_Size)
		{
			m_Index = 0;
		}
	}

	void process(uint16_t index)
	{
		ScriptElement el = m_Script[m_Index];
		switch (el.instruction)
		{
		case ScriptInstruction::MSG:
			//test by making listID simple char
			EngineLog((char)el.info.msgInfo.listID);
			m_Index++;
			break;
		case ScriptInstruction::JMP:
			m_Index = el.info.jmpInfo.line;
			break;
		case ScriptInstruction::FREEZE_OBJECT:
			m_MessageUpdate(Message::DEACTIVATE, id());
			m_Index++;
			break;
		case ScriptInstruction::UNFREEZE_OBJECT:
			m_MessageUpdate(Message::ACTIVATE, id());
			m_Index++;
			break;
		}
	}
private:
	Script m_Script;
	uint16_t m_Index = 0;
	uint16_t m_Size;
	std::function <void(Message message, unsigned int id)> m_MessageUpdate;
};

#endif
