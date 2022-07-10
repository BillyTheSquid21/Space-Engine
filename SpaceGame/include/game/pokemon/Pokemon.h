#pragma once
#ifndef POKEMON_H
#define POKEMON_H

#include <stdint.h>
#include <array>
#include <string>

//Define types
enum class PokemonType : uint8_t
{
	Normal, 
	Fire, 
	Water, 
	Grass, 
	Electric, 
	Ice, 
	Fighting, 
	Poison, 
	Ground, 
	Flying, 
	Psychic, 
	Bug, 
	Rock, 
	Ghost, 
	Dark, 
	Dragon, 
	Steel, 
	Fairy,
	None
};

enum class StatusCondition : int8_t
{
	None, Poison, Sleep, Paralysis, Burn, Confusion,
};

enum class PokemonNature : int8_t
{
	None,
	Hardy,
	Bold,
	Modest,
	Calm,
	Timid,
	Lonely,
	Docile,
	Mild,
	Gentle,
	Hasty,
	Adamant,
	Impish,
	Bashful,
	Careful,
	Rash,
	Jolly,
	Naughty,
	Lax,
	Quirky,
	Naive,
	Brave,
	Relaxed,
	Quiet,
	Sassy,
	Serious
};

//Define moves
struct PokemonMove
{ 
	std::string identifier = " ";
	//If null move is null
	uint16_t id = 0;
	PokemonType type = PokemonType::None;
	uint8_t damageAcc = 100;
	uint8_t statusAcc = 30;
	uint8_t damage = 0;
	StatusCondition status = StatusCondition::None;
	uint8_t additionalEffect = 0; //If a logically different effect is part of the move = 0 if none
	int16_t additionalData = 0;
};

//Slot the move fits in - can be used as direct index into move array
enum class MoveSlot : int8_t
{
	SLOT_1 = 0, SLOT_2 = 1, SLOT_3 = 2, SLOT_4 = 3, SLOT_NULL = 4
};

enum class MoveCategory : int8_t
{
	Physical, Special, Status
};

struct PokemonStats
{
	int16_t hp = 23;
	int16_t attack = 14;
	int16_t defense = 13;
	int16_t spAttack = 14;
	int16_t spDefense = 14;
	int16_t speed = 17;
};

//Define pokemon struct
struct Pokemon
{
	//Identifying info
	std::string nickname = "#default"; //if #default lookup name
	int16_t id = -1; //If id is negative, pokemon is null
	PokemonNature nature = PokemonNature::None;
	//Battle info
	PokemonType primaryType = PokemonType::Normal;
	PokemonType secondaryType = PokemonType::None;
	int16_t level = 50;
	int16_t health = 23;
	StatusCondition condition = StatusCondition::None;
	PokemonStats stats;
	//Moves
	std::array<PokemonMove, 4> moves;
	//Other
	int8_t storage = 0; //Can be used for anything depending on context (eg for sleep acts as timer for that pokemon)
};

typedef std::array<Pokemon, 6> Party;

#endif