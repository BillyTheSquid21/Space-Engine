#pragma once
#ifndef TURN_HPP
#define TURN_HPP

#include <stdint.h>
#include "game/pokemon/Pokemon.h"
#include "game/pokemon/StatStages.hpp"

struct TurnMove
{
	//Move component
	uint16_t id = 0;
	PokemonType type = PokemonType::None;
	DamageType damageType;
	uint8_t accuracy = 100;
	uint8_t damage = 0;
	int16_t effectID = 1;
	int8_t effectData = 0;
	bool skipAccuracy = false; //Skips the damage calculation
};

//Determines if a pokemon is in a unique state (eg semi invulnerable from fly)
enum class PokemonMoveState
{
	Normal, Flying, Digging, Diving
};

struct TurnData
{
	TurnMove move;

	//Attack component
	Pokemon* origin;
	CurrentStages* originStages;
	Pokemon* target;
	CurrentStages* targetStages;
	PokemonMoveState* originState;
	PokemonMoveState* targetState;

	std::string message;
};

#endif