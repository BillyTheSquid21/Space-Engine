#pragma once
#ifndef POKEMON_BATTLE_H
#define POKEMON_BATTLE_H

#include "utility/SGUtil.h"
#include "game/pokemon/Pokemon.h"
#include "game/utility/Random.hpp"

#define MOVE_QUEUE_LENGTH 30
#define BATTLE_PROBABILITY_MAX 100

void ExecuteAttack(Pokemon& attacker, Pokemon& target, PokemonMove move);

static struct MoveInfo
{
	PokemonMove move;
	bool wasPlayer;
};

class PokemonBattle
{
public:
	PokemonBattle(Party playerParty, Party enemyParty) { m_PlayerParty = playerParty; m_EnemyParty = enemyParty; m_Random.seed(0.0f, BATTLE_PROBABILITY_MAX);};
private:

	void run();

	//Stack array containing next moves
	MoveInfo m_MoveQueue[MOVE_QUEUE_LENGTH];
	unsigned int m_MoveQueueIndex = 0;

	//Parties
	Party m_PlayerParty; Party m_EnemyParty;
	unsigned int m_PlayerActivePkm = 0; unsigned int m_EnemyActivePkm = 0;

	RandomContainer m_Random;
};

#endif
