#pragma once
#ifndef POKEMON_BATTLE_H
#define POKEMON_BATTLE_H

#include "utility/SGUtil.h"
#include "game/pokemon/Pokemon.h"
#include "game/utility/Random.hpp"

#define MOVE_QUEUE_LENGTH 30
#define BATTLE_PROBABILITY_MAX 10000

static void ExecuteAttack(Pokemon& attacker, Pokemon& target, PokemonMove move);

static int16_t CalculateDamage(Pokemon& attacker, Pokemon& target, PokemonMove move);
static void ProcessEndTurnStatus(Pokemon& pokemon);
static bool ProcessStatus(Pokemon& pokemon);

#define STAGES_COUNT 13
static enum class Stage : int8_t
{
	STAGE_N6 = 0, 
	STAGE_N5 = 1, 
	STAGE_N4 = 2, 
	STAGE_N3 = 3, 
	STAGE_N2 = 4, 
	STAGE_N1 = 5, 
	STAGE_0 =  6, 
	STAGE_P1 = 7, 
	STAGE_P2 = 8, 
	STAGE_P3 = 9, 
	STAGE_P4 = 10, 
	STAGE_P5 = 11, 
	STAGE_P6 = 12
};

static constexpr float StatStageLookup[STAGES_COUNT]
{
	1.0f/4.0f, 2.0f/7.0f, 2.0f/6.0f, 2.0f/5.0f, 2.0f/4.0f, 2.0f/3.0f,
	1.0f,
	3.0f/2.0f, 4.0f/2.0f, 5.0f/2.0f, 6.0f/2.0f, 7.0f/2.0f, 8.0f/2.0f
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
static float LookupStageMultiplier(Stage stage);

struct CurrentStages
{
	Stage attackStage = Stage::STAGE_0;
	Stage defenseStage = Stage::STAGE_0;
	Stage spAttackStage = Stage::STAGE_0;
	Stage spDefenseStage = Stage::STAGE_0;
	Stage speedStage = Stage::STAGE_0;
	Stage critStage = Stage::STAGE_0;
};

class PokemonBattle
{
public:
	PokemonBattle() { random.seed(0.0f, BATTLE_PROBABILITY_MAX); }
	
	void setParties(Party playerParty, Party enemyParty) { m_PartyA = playerParty; m_PartyB = enemyParty; };
	void run(int move);

	int16_t getHealthA() { return m_PartyA[m_ActivePkmA].health; }
	int16_t getHealthB() { return m_PartyB[m_ActivePkmB].health; }
	uint8_t getStatusA() { return (uint8_t)m_PartyA[m_ActivePkmA].condition; }
	uint8_t getStatusB() { return (uint8_t)m_PartyB[m_ActivePkmB].condition; }

	static RandomContainer random;
private:

	void nextMove();
	
	static enum class Team : uint8_t
	{
		A, B
	};

	struct MoveInfo
	{
		PokemonMove move;
		Team team;
	};

	//Stack array containing next moves
	MoveInfo m_MoveQueue[MOVE_QUEUE_LENGTH];
	unsigned int m_MoveQueueIndex = 0;

	//Pokemon
	Party m_PartyA; Party m_PartyB;
	unsigned int m_ActivePkmA = 0; unsigned int m_ActivePkmB = 0;

	CurrentStages m_StagesA; CurrentStages m_StagesB;
};

#endif
