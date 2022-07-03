#pragma once
#ifndef POKEMON_BATTLE_H
#define POKEMON_BATTLE_H

#include "utility/SGUtil.h"

#include "game/pokemon/Pokemon.h"
#include "game/utility/Random.hpp"

#include <chrono>

#define MOVE_QUEUE_LENGTH 3
#define BATTLE_PROBABILITY_MAX 10000
#define STAGES_COUNT 13
#define TYPE_COUNT 18

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

struct CurrentStages
{
	Stage attackStage = Stage::STAGE_0;
	Stage defenseStage = Stage::STAGE_0;
	Stage spAttackStage = Stage::STAGE_0;
	Stage spDefenseStage = Stage::STAGE_0;
	Stage speedStage = Stage::STAGE_0;
	Stage critStage = Stage::STAGE_0;
};

static uint8_t LookupTypeMultiplier(PokemonType attacking, PokemonType defending);
static float LookupStageMultiplier(Stage stage);
static void ExecuteAttack(Pokemon& attacker, Pokemon& target, PokemonMove move, CurrentStages& attackerStage, CurrentStages& defenderStage);
static int16_t CalculateDamage(Pokemon& attacker, Pokemon& target, PokemonMove move, CurrentStages& attackerStage, CurrentStages& defenderStage);
static bool ProcessStatus(Pokemon& pokemon);
static void ProcessEndTurnStatus(Pokemon& pokemon);

class MoveQueue
{
public:
	MoveQueue() { random.seed(0.0f, BATTLE_PROBABILITY_MAX); }
	void queueMove(PokemonMove move, Pokemon* origin, Pokemon* target, CurrentStages* originStages, CurrentStages* targetStages);
	void queueMove(PokemonMove move, Pokemon* origin, Pokemon* target, CurrentStages* originStages, CurrentStages* targetStages, unsigned int turnIndex);
	void processTurn();

	static const int POKEMON_COUNT = 2;
	struct MoveTurn
	{
		struct Combined
		{
			PokemonMove move;
			Pokemon* origin;
			CurrentStages* originStages;
			Pokemon* target;
			CurrentStages* targetStages;
		};
		Combined moves[POKEMON_COUNT];
		uint8_t size = 0;
	};
	static RandomContainer random;
private:
	MoveTurn m_MoveQueue[MOVE_QUEUE_LENGTH]; //Array of how many moves to look ahead
	unsigned int m_QueueIndex = 0; //Rolls around to make queue array circular
};

static bool SortBySpeed(const MoveQueue::MoveTurn::Combined& lhs, const MoveQueue::MoveTurn::Combined& rhs);

//Static class for text buffer
class BattleTextBuffer
{
public:
	static void pushText(std::string text);
	static void clearText() { m_Line1 = ""; m_Line2 = ""; m_CurrentLine = 1; }
	static std::string m_Line1;
	static std::string m_Line2;
private:
	static int m_CurrentLine;
};

class PokemonBattle
{
public:
	PokemonBattle() { random.seed(0.0f, BATTLE_PROBABILITY_MAX); }
	void linkProgressButtons(bool* pressE, bool* pressX) { s_Progress[0] = pressE; s_Progress[1] = pressX; }

	void setParties(Party playerParty, Party enemyParty) { m_PartyA = playerParty; m_PartyB = enemyParty; };
	void run(MoveSlot move);

	int16_t getHealthA() { return m_PartyA[m_ActivePkmA].health; }
	int16_t getHealthB() { return m_PartyB[m_ActivePkmB].health; }
	uint8_t getStatusA() { return (uint8_t)m_PartyA[m_ActivePkmA].condition; }
	uint8_t getStatusB() { return (uint8_t)m_PartyB[m_ActivePkmB].condition; }
	std::string getNameA() { return m_PartyA[m_ActivePkmA].nickname; }
	std::string getNameB() { return m_PartyB[m_ActivePkmB].nickname; }

	bool checkMoveValid(MoveSlot slot) { if (m_PartyA[m_ActivePkmA].moves[(int)slot].type == PokemonType::Null) { return false; } return true; }
	bool isUpdating() { return m_IsUpdating; }
	
	//Only to be called from battle thread - otherwise will likely lock
	static void awaitInput();
	static RandomContainer random;
private:

	void nextMove();
	bool checkWin();
	
	static enum class Team : uint8_t
	{
		A, B
	};

	//Stack array containing next moves
	MoveQueue m_MoveQueue;

	//Bool ptr to next button
	static bool* s_Progress[2];

	//Pokemon
	Party m_PartyA; Party m_PartyB;
	unsigned int m_ActivePkmA = 0; unsigned int m_ActivePkmB = 0;
	MoveSlot m_NextMoveA = MoveSlot::SLOT_NULL;
	MoveSlot m_NextMoveB = MoveSlot::SLOT_NULL;
	CurrentStages m_StagesA; CurrentStages m_StagesB;

	//Thread
	std::atomic<bool> m_IsUpdating = false;
};

#endif
