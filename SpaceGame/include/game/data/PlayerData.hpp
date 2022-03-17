#pragma once
#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

#include <vector>
#include <string>

//Struct for storing savefile specific data used ingame
struct GameData
{
	std::string playerName;
	std::vector<bool> flags;
};

#endif