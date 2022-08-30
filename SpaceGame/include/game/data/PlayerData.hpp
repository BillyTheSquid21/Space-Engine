#pragma once
#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

#include "vector"
#include "string"
#include "game/level/World.h"
#include "game/pokemon/Pokemon.h"
#include "game/items/Items.hpp"
#include "game/utility/Flags.hpp"
#include "game/objects/OverworldSprite.h"

//Struct for storing savefile specific data used ingame
struct PlayerData
{
	//Player data
	std::string playerName;
	unsigned int playerNumber = 0;
	World::Tile tile = { 12, 3 };
	World::WorldHeight height = World::WorldHeight::F0;
	World::LevelID id = World::LevelID::LEVEL_ENTRY;
	Party playerParty;
	PlayerBag bag;

	//Game data
	FlagArray flags = {};
	double timePlayed = 0.0;
};

#endif