#pragma once
#ifndef POKEMON_H
#define POKEMON_H

#include <stdint.h>
#include <array>

//Define types
enum class PokemonType : uint8_t
{
	Normal, Fire, Water, Grass, Electric, Ice, Fighting, Poison, Ground, Flying, 
	Psychic, Bug, Rock, Ghost, Dark, Dragon, Steel, Fairy, Null
};

enum class StatusCondition : uint8_t
{
	None, Poison, Sleep, Paralysis, Burn, Confusion,
};

//Define moves
struct PokemonMove
{ 
	//If null move is null
	uint8_t id;
	PokemonType type = PokemonType::Null;
	uint8_t damageAcc = 100;
	uint8_t statusAcc = 30;
	uint8_t damage;
	StatusCondition status = StatusCondition::None;
};

//Define pokemon struct
struct Pokemon
{
	int16_t id = -1; //If id is negative, pokemon is null
	PokemonType primaryType = PokemonType::Normal;
	PokemonType secondaryType = PokemonType::Null;
	int16_t level = 5;
	int16_t health = 23;
	StatusCondition condition = StatusCondition::None;
	//Stats
	int16_t hp = 23;
	int16_t attack = 14;
	int16_t defense = 13;
	int16_t spAttack = 14;
	int16_t spDefense = 14;
	int16_t speed = 17;
	//Moves
	std::array<PokemonMove, 4> moves;
	//Other
	int8_t storage = 0; //Can be used for anything depending on context (eg for sleep acts as timer for that pokemon)
};

typedef std::array<Pokemon, 6> Party;


#endif