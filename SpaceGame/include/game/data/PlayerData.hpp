#pragma once
#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

#include <vector>
#include <string>
#include "game/level/World.h"
#include "game/pokemon/Pokemon.h"
#include "game/utility/Flags.hpp"
#include "game/objects/OverworldSprite.h"

//Struct for storing savefile specific data used ingame
struct GameData
{
	//Player data
	std::string playerName;
	unsigned int playerNumber = 0;
	std::shared_ptr<OvSpr_RunningSprite> playerData; //Can be added to the object manager
	Party playerParty;

	//Game data
	FlagArray flags;
	double timePlayed = 0.0;
};

#endif