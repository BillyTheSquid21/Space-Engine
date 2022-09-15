#include "game/objects/ScriptParse.h"

ScriptParse::ScriptWrapper ScriptParse::ParseScriptFromText(std::string scriptName)
{
	using namespace ScriptParse;
	std::string path = "res/scripts/" + scriptName;
	std::ifstream* ifs = new std::ifstream(path);
	std::string tmp;

	//String separation
	std::stringstream stringsBuffer;
	std::string segment;

	stringsBuffer << ifs->rdbuf();
	ifs->close();
	delete ifs;

	//Parse into list of segments - should be fast enough for most use cases
	std::vector<std::string> seglist;
	while (std::getline(stringsBuffer, segment, '\n'))
	{
		if (segment[0] != '#')
		{
			seglist.push_back(segment);
		}
	}

	//Parse each segment - assumes all lines are valid
	Script script(new ScriptElement[seglist.size()]);
	std::string instrArr[MAX_INFO];
	int index = 0;
	for (int i = 0; i < seglist.size(); i++)
	{
		std::string currentSegment = seglist[i];
		stringsBuffer.clear();
		stringsBuffer.str(currentSegment);
		index = 0;
		while (std::getline(stringsBuffer, segment, ' '))
		{
			if (segment[0] != '[')
			{
				instrArr[index] = segment;
				index++;
			}
		}

		//Process instructions:
		//Check if instruction is string or numerical:
		ScriptInstruction instruction;
		if (instrArr[0][0] == '-')
		{
			instruction = GetInstruction(instrArr[0]);
		}
		else
		{
			instruction = (ScriptInstruction)strtoul(instrArr[0].c_str(), nullptr, 10);
		}
		script[i].instruction = instruction;
		//Based on instruction process attatched data
		ScriptParse::ProcessInstructionInfo(instruction, instrArr, script[i]);
	}

	return {script, seglist.size()};
}

//instrs
ScriptInstruction ScriptParse::GetInstruction(std::string instr)
{
	if (instr == "-NO_OP")
	{
		return ScriptInstruction::NO_OP;
	}
	else if (instr == "-BLANK_OP")
	{
		return ScriptInstruction::BLANK_OP;
	}
	else if (instr == "-SET_FLAG")
	{
		return ScriptInstruction::SET_FLAG;
	}
	else if (instr == "-JMP")
	{
		return ScriptInstruction::JMP;
	}
	else if (instr == "-JMP_IF")
	{
		return ScriptInstruction::JMP_IF;
	}
	else if (instr == "-WAIT_SEC")
	{
		return ScriptInstruction::WAIT_SEC;
	}
	else if (instr == "-PLAYER_LOCK")
	{
		return ScriptInstruction::PLAYER_LOCK;
	}
	else if (instr == "-PLAYER_FACE")
	{
		return ScriptInstruction::PLAYER_FACE;
	}
	else if (instr == "-PLAYER_WALK")
	{
		return ScriptInstruction::PLAYER_WALK;
	}
	else if (instr == "-PLAYER_RUN")
	{
		return ScriptInstruction::PLAYER_RUN;
	}
	else if (instr == "-OPEN_MSG_BOX")
	{
		return ScriptInstruction::OPEN_MSG_BOX;
	}
	else if (instr == "-CLOSE_MSG_BOX")
	{
		return ScriptInstruction::CLOSE_MSG_BOX;
	}
	else if (instr == "-MSG")
	{
		return ScriptInstruction::MSG;
	}
	else if (instr == "-CLEAR_TEXT")
	{
		return ScriptInstruction::CLEAR_TEXT;
	}
	else if (instr == "-GIVE_ITEM")
	{
		return ScriptInstruction::GIVE_ITEM;
	}
	else if (instr == "-HEAL_PARTY")
	{
		return ScriptInstruction::HEAL_PARTY;
	}
	else if (instr == "-TAKE_ITEM")
	{
		return ScriptInstruction::TAKE_ITEM;
	}
	else if (instr == "-WARP_PLAYER")
	{
		return ScriptInstruction::WARP_PLAYER;
	}
	else if (instr == "-WAIT_INPUT")
	{
		return ScriptInstruction::WAIT_INPUT;
	}
	else if (instr == "-WAIT_FOR")
	{
		return ScriptInstruction::WAIT_FOR;
	}
	else if (instr == "-SET_PLAYER_TILE")
	{
		return ScriptInstruction::SET_PLAYER_TILE;
	}
	else if (instr == "-PLAYER_WALK_TO_TILE")
	{
		return ScriptInstruction::PLAYER_WALK_TO_TILE;
	}
	else if (instr == "-PLAYER_RUN_TO_TILE")
	{
		return ScriptInstruction::PLAYER_RUN_TO_TILE;
	}
	else if (instr == "-FACE_PLAYER")
	{
		return ScriptInstruction::FACE_PLAYER;
	}
	else if (instr == "-FREEZE_OBJECT")
	{
		return ScriptInstruction::FREEZE_OBJECT;
	}
	else if (instr == "-FACE_WITH_PLAYER")
	{
		return ScriptInstruction::FACE_WITH_PLAYER;
	}
	else if (instr == "-NPC_FACE")
	{
		return ScriptInstruction::NPC_FACE;
	}
	else if (instr == "-NPC_WALK")
	{
		return ScriptInstruction::NPC_WALK;
	}
	else if (instr == "-NPC_RUN")
	{
		return ScriptInstruction::NPC_RUN;
	}
	else if (instr == "-NPC_AND_PLAYER_WALK")
	{
		return ScriptInstruction::NPC_AND_PLAYER_WALK;
	}
	else if (instr == "-NPC_WALK_TO_TILE")
	{
		return ScriptInstruction::NPC_WALK_TO_TILE;
	}
	else if (instr == "-NPC_RUN_TO_TILE")
	{
		return ScriptInstruction::NPC_RUN_TO_TILE;
	}
	else if (instr == "-SHOW_SPRITE")
	{
		return ScriptInstruction::SHOW_SPRITE;
	}
	else if (instr == "-WAIT_INTERACT")
	{
		return ScriptInstruction::WAIT_INTERACT;
	}
	return ScriptInstruction::CORE_MAX;
}

//process instrs
void ScriptParse::ProcessInstructionInfo(ScriptInstruction instr, std::string(&instrArr)[MAX_INFO], ScriptElement& element)
{
	switch (instr)
	{
	case ScriptInstruction::SET_FLAG:
		PSS_SET_FLAG(instrArr, element);
		return;
	case ScriptInstruction::JMP:
		PSS_JMP(instrArr, element);
		return;
	case ScriptInstruction::JMP_IF:
		PSS_JMP_IF(instrArr, element);
		return;
	case ScriptInstruction::PLAYER_LOCK:
		PSS_BOOL(instrArr, element);
		return;
	case ScriptInstruction::PLAYER_FACE:
		PSS_CGE_DIR(instrArr, element);
		return;
	case ScriptInstruction::MSG:
		PSS_MSG(instrArr, element);
		return;
	case ScriptInstruction::WAIT_FOR:
		PSS_WAIT(instrArr, element);
		return;
	case ScriptInstruction::GIVE_ITEM:
		PSS_GIVE_TAKE_ITEM(instrArr, element);
		return;
	case ScriptInstruction::TAKE_ITEM:
		PSS_GIVE_TAKE_ITEM(instrArr, element);
		return;
	case ScriptInstruction::WARP_PLAYER:
		PSS_WARP(instrArr, element);
		return;
	case ScriptInstruction::SET_PLAYER_TILE:
		PSS_SET_TILE(instrArr, element);
		return;
	case ScriptInstruction::PLAYER_WALK_TO_TILE:
		PSS_SET_TILE(instrArr, element);
		return;
	case ScriptInstruction::PLAYER_RUN_TO_TILE:
		PSS_SET_TILE(instrArr, element);
		return;
	case ScriptInstruction::NPC_FACE:
		PSS_CGE_DIR(instrArr, element);
		return;
	case ScriptInstruction::FREEZE_OBJECT:
		PSS_BOOL(instrArr, element);
		return;
	case ScriptInstruction::SHOW_SPRITE:
		PSS_BOOL(instrArr, element);
		return;
	case ScriptInstruction::NPC_WALK_TO_TILE:
		PSS_SET_TILE(instrArr, element);
		return;
	case ScriptInstruction::NPC_RUN_TO_TILE:
		PSS_SET_TILE(instrArr, element);
		return;
	default:
		element.info.clear = 0;
		return;
	}
}

void ScriptParse::PSS_SET_FLAG(std::string(&instrArr)[MAX_INFO], ScriptElement& element)
{
	element.info.flgInfo.flagLoc = (uint16_t)strtoul(instrArr[1].c_str(), nullptr, 16);
	if (instrArr[2][0] != '-')
	{
		element.info.flgInfo.state = (uint8_t)strtoul(instrArr[2].c_str(), nullptr, 10);
		return;
	}
	else if (instrArr[2] == "true")
	{
		element.info.flgInfo.state = 1;
		return;
	}
	element.info.flgInfo.state = 0; //defaults to false
	return;
}

void ScriptParse::PSS_JMP(std::string(&instrArr)[MAX_INFO], ScriptElement& element)
{
	element.info.jmpInfo.line = (uint16_t)strtoul(instrArr[1].c_str(), nullptr, 10);
}

void ScriptParse::PSS_JMP_IF(std::string(&instrArr)[MAX_INFO], ScriptElement& element)
{
	element.info.jmpIfInfo.flagLoc = (uint16_t)strtoul(instrArr[1].c_str(), nullptr, 16);
	element.info.jmpIfInfo.line = (uint16_t)strtoul(instrArr[2].c_str(), nullptr, 10);
}

void ScriptParse::PSS_BOOL(std::string(&instrArr)[MAX_INFO], ScriptElement& element)
{
	if (instrArr[1][0] != '-')
	{
		element.info.boolInfo.state = (uint8_t)strtoul(instrArr[1].c_str(), nullptr, 10);
		return;
	}
	else if (instrArr[1] == "true")
	{
		element.info.boolInfo.state = 1;
		return;
	}
	element.info.boolInfo.state = 0;
}

void ScriptParse::PSS_MSG(std::string(&instrArr)[MAX_INFO], ScriptElement& element)
{
	element.info.msgInfo.listID = (uint16_t)strtoul(instrArr[1].c_str(), nullptr, 10);
	element.info.msgInfo.textEntry = (uint16_t)strtoul(instrArr[2].c_str(), nullptr, 10);
}

void ScriptParse::PSS_GIVE_TAKE_ITEM(std::string(&instrArr)[MAX_INFO], ScriptElement& element)
{
	element.info.itemInfo.itemID = (uint16_t)strtoul(instrArr[1].c_str(), nullptr, 10);
	element.info.itemInfo.quantity = (uint16_t)strtoul(instrArr[2].c_str(), nullptr, 10);
}

void ScriptParse::PSS_CGE_DIR(std::string(&instrArr)[MAX_INFO], ScriptElement& element)
{
	if (instrArr[1][0] == '-')
	{
		element.info.dirInfo.direction = (uint8_t)World::GetDirection(instrArr[1].substr(1, instrArr[1].size()));
		return;
	}
	else
	{
		element.info.dirInfo.direction = (uint8_t)strtoul(instrArr[1].c_str(), nullptr, 10);
		return;
	}
}

void ScriptParse::PSS_WAIT(std::string(&instrArr)[MAX_INFO], ScriptElement& element)
{
	element.info.waitInfo.millis = (uint32_t)strtoul(instrArr[1].c_str(), nullptr, 10);
}

void ScriptParse::PSS_SET_TILE(std::string(&instrArr)[MAX_INFO], ScriptElement& element)
{
	element.info.tileInfo.x = (uint16_t)strtoul(instrArr[1].c_str(), nullptr, 10); //Maxes out at 4096 due to reduced bits
	element.info.tileInfo.z = (uint16_t)strtoul(instrArr[2].c_str(), nullptr, 10);
	element.info.tileInfo.h = (int8_t)strtoul(instrArr[3].c_str(), nullptr, 10);
}

void ScriptParse::PSS_WARP(std::string(&instrArr)[MAX_INFO], ScriptElement& element)
{
	element.info.warpInfo.origin = (int16_t)strtoul(instrArr[1].c_str(), nullptr, 10);
	element.info.warpInfo.dest = (int16_t)strtoul(instrArr[2].c_str(), nullptr, 10);
}
