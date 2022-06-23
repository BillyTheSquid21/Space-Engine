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

struct MoveInfo
{
	PokemonMove move;
	bool wasPlayer;
};

static enum class Stage : int8_t
{
	STAGE_N6 = -6, 
	STAGE_N5 = -5, 
	STAGE_N4 = -4, 
	STAGE_N3 = -3, 
	STAGE_N2 = -2, 
	STAGE_N1 = -1, 
	STAGE_0 = 0, 
	STAGE_P1 = 1, 
	STAGE_P2 = 2, 
	STAGE_P3 = 3, 
	STAGE_P4 = 4, 
	STAGE_P5 = 5, 
	STAGE_P6 = 6
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

class PokemonBattle
{
public:
	PokemonBattle() { random.seed(0.0f, BATTLE_PROBABILITY_MAX); }
	
	void setParties(Party playerParty, Party enemyParty) { m_PlayerParty = playerParty; m_EnemyParty = enemyParty; };
	void run();

	static RandomContainer random;
private:
	//Stack array containing next moves
	MoveInfo m_MoveQueue[MOVE_QUEUE_LENGTH];
	unsigned int m_MoveQueueIndex = 0;

	//Pokemon
	Party m_PlayerParty; Party m_EnemyParty;
	unsigned int m_PlayerActivePkm = 0; unsigned int m_EnemyActivePkm = 0;

	CurrentStages m_PlayerStages; CurrentStages m_EnemyStages;
};

#endif