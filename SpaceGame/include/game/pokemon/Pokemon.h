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

//Type effectiveness lookup for ATTACK - don't need to store defensive info
//Stored in order the types are stored above and so can be used as keys
//0 is no effect, 1 is half power, 2 is normal power, 4 is 2x power (value / 2 to get multiplier)
#define TYPE_COUNT 18

static const uint8_t FairyEff[TYPE_COUNT]
{
	2,1,2,2,2,2,4,1,2,2,2,2,2,2,4,4,1,2
};
static const uint8_t SteelEff[TYPE_COUNT]
{
	2,1,1,2,1,4,2,2,2,2,2,2,4,2,2,2,1,4
};
static const uint8_t DragonEff[TYPE_COUNT]
{
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,1,0
};
static const uint8_t DarkEff[TYPE_COUNT]
{
	2,2,2,2,2,2,1,2,2,2,4,2,2,4,1,2,2,1
};
static const uint8_t GhostEff[TYPE_COUNT]
{
	0,2,2,2,2,2,2,2,2,2,4,2,2,4,1,2,2,2
};
static const uint8_t RockEff[TYPE_COUNT]
{
	2,4,2,2,2,4,1,2,1,4,2,4,2,2,2,2,1,2
};
static const uint8_t BugEff[TYPE_COUNT]
{
	2,1,2,4,2,2,1,1,2,1,4,2,2,1,4,2,1,1
};
static const uint8_t PsychicEff[TYPE_COUNT]
{
	2,2,2,2,2,2,4,4,2,2,1,2,2,2,0,2,1,2
};
static const uint8_t FlyingEff[TYPE_COUNT]
{
	2,2,2,4,1,2,4,2,2,2,2,4,1,2,2,2,1,2
};
static const uint8_t GroundEff[TYPE_COUNT]
{
	2,4,2,1,4,2,2,4,2,0,2,1,4,2,2,2,4,2
};
static const uint8_t PoisonEff[TYPE_COUNT]
{
	2,2,2,4,2,2,2,1,1,2,2,2,1,1,2,2,0,4
};
static const uint8_t FightingEff[TYPE_COUNT]
{
	4,2,2,2,2,4,2,1,2,1,1,1,4,0,4,2,4,1
};
static const uint8_t NormalEff[TYPE_COUNT]
{
	2,2,2,2,2,2,2,2,2,2,2,2,1,0,2,2,1,1
};
static const uint8_t FireEff[TYPE_COUNT]
{
	2,1,1,4,2,4,2,2,2,2,2,4,1,2,2,1,4,2
};
static const uint8_t WaterEff[TYPE_COUNT]
{
	2,4,1,1,2,2,2,2,4,2,2,2,4,2,2,1,2,2
};
static const uint8_t GrassEff[TYPE_COUNT]
{
	2,1,4,1,2,2,2,1,4,1,2,1,4,2,2,1,1,2
};
static const uint8_t ElectricEff[TYPE_COUNT]
{
	2,2,4,1,1,2,2,2,0,4,2,2,2,2,2,1,2,2
};
static const uint8_t IceEff[TYPE_COUNT]
{
	2,1,1,4,2,1,2,2,4,4,2,2,2,2,2,4,1,1
};

static const uint8_t* TypeLookup[TYPE_COUNT]
{
	NormalEff, FireEff, WaterEff, GrassEff, ElectricEff, IceEff, FightingEff, PoisonEff, GroundEff, FlyingEff,
	PsychicEff, BugEff, RockEff, GhostEff, DarkEff, DragonEff, SteelEff, FairyEff
};

static uint8_t LookupTypeMultiplier(PokemonType attacking, PokemonType defending);

//Define moves
struct PokemonMove
{ 
	//If null move is null
	uint8_t id;
	PokemonType type = PokemonType::Null;
	uint8_t damageAcc = 100;
	uint8_t statusAcc = 30;
	uint8_t damage;
	StatusCondition status = StatusCondition::Paralysis;
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