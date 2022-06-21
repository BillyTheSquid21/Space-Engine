#ifndef GAME_SCRIPT_PARSE
#define GAME_SCRIPT_PARSE

#include "game/objects/script.hpp"
#include "game/level/World.h"

///Script parsing
///
///Structure (file ends in .sgs)
/// 
///
///[n] -INSTR -PARAM1 -PARAM2 ... 
///[n+1] INSTR PARAM1 PARAM2 ...
///#THIS IS A COMMENT
/// 
///Explained (should be easy):
///[n] - line number surrounded by square brackets to separate
///-INSTR - String representative of desired instruction
///-PARAM - String representative of parameter that is converted to number (not all params have a string equivalent)
///INSTR - Number representative of desired instruction
///PARAM - Number representative of desired parameter
/// 

//Script parser
namespace ScriptParse
{
	struct ScriptWrapper
	{
		Script script;
		size_t size;
	};

	ScriptWrapper ParseScriptFromText(std::string scriptName);
	constexpr int MAX_INFO = 4; //Most instructions that could be in the array

	inline static ScriptInstruction GetInstruction(std::string instr);
	inline static void ProcessInstructionInfo(ScriptInstruction instr, std::string(&instrArr)[MAX_INFO], ScriptElement& element);

	//Process particular instrs - not all instrs have info attatched
	inline static void PSS_SET_FLAG(std::string(&instrArr)[MAX_INFO], ScriptElement& element);
	inline static void PSS_JMP(std::string(&instrArr)[MAX_INFO], ScriptElement& element);
	inline static void PSS_JMP_IF(std::string(&instrArr)[MAX_INFO], ScriptElement& element);
	inline static void PSS_LOCK(std::string(&instrArr)[MAX_INFO], ScriptElement& element);
	inline static void PSS_MSG(std::string(&instrArr)[MAX_INFO], ScriptElement& element);
	inline static void PSS_GIVE_TAKE_ITEM(std::string(&instrArr)[MAX_INFO], ScriptElement& element);
	inline static void PSS_CGE_DIR(std::string (&instrArr)[MAX_INFO], ScriptElement& element);
}

#endif