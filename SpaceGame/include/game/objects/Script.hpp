#pragma once
#ifndef GAME_SCRIPT_H
#define GAME_SCRIPT_H

#include <stdint.h>
#include <string>
#include <memory>
#include <functional>

#include "game/utility/Flags.hpp"

//Instructions
enum class ScriptInstruction : uint32_t
{
	//Core
	NO_OP, SET_FLAG, JMP, JMP_IF, WAIT_SEC, LOCK_PLAYER, PLAYER_FACE, PLAYER_WALK, PLAYER_RUN, 
	OPEN_MSG_BOX, CLOSE_MSG_BOX, MSG, CLEAR_TEXT, GIVE_ITEM, TAKE_ITEM, WAIT_INPUT,
	WARP_PLAYER, SET_PLAYER_TILE, //TODO - implement

	CORE_MAX, //max to give return term - TODO define each exactly so can guarantee CORE_MAX is bigger
	
	//NPC aimed
	FREEZE_OBJECT, FACE_PLAYER, NPC_FACE, NPC_WALK, NPC_RUN, WAIT_INTERACT, 
	NPC_AND_PLAYER_WALK, SHOW_SPRITE, 
};

//NOTE: flags must be written in hex in files (0xFFFFFF syntax)

//Message info - using std ints to guarantee bytes - all on byte borders (nothing bitwise)
struct MSG_INFO
{
	//Which list of text to take from
	uint32_t listID : 16;
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

//Direction info - walk_in_dir requires changing dir here and has not got its own data
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

//Warp info
struct WARP_INFO
{
	uint32_t origin : 16;
	uint32_t dest : 16;
};

//Tile info
struct TILE_INFO
{
	uint32_t x : 16;
	uint32_t y : 16;
};

//Bool (general purpose)
struct BOOL_INFO
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
		BOOL_INFO boolInfo;
		WARP_INFO warpInfo;
		TILE_INFO tileInfo;
		uint32_t clear;
	};
};

struct ScriptElement
{
	ScriptInstruction instruction;
	InstructionInfo info;
};

typedef std::shared_ptr<ScriptElement[]> Script;

#endif
