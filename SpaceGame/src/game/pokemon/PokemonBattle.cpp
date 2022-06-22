#include "game/pokemon/PokemonBattle.h"

void ExecuteAttack(Pokemon& attacker, Pokemon& target, PokemonMove move)
{
	int16_t damage = (move.damage * attacker.attack) / target.defense;
	target.hp -= damage;
}

void PokemonBattle::run()
{
	//Await player input, add to queue
	m_MoveQueueIndex++;
	
	//Await enemy input, add to queue
	m_MoveQueueIndex++;

	//Execute top move
	if (m_MoveQueue[m_MoveQueueIndex - 1].wasPlayer)
	{
		ExecuteAttack(m_PlayerParty[m_PlayerActivePkm], m_EnemyParty[m_EnemyActivePkm], m_MoveQueue[m_MoveQueueIndex - 1].move);
	}
	else
	{
		ExecuteAttack(m_EnemyParty[m_EnemyActivePkm], m_PlayerParty[m_PlayerActivePkm], m_MoveQueue[m_MoveQueueIndex - 1].move);
	}
	m_MoveQueueIndex--;

	//Check if won
	if (m_PlayerParty[m_PlayerActivePkm].hp < 0)
	{
		EngineLog("Player Lost!");
	}
	else if (m_EnemyParty[m_EnemyActivePkm].hp < 0)
	{
		EngineLog("Enemy Lost!");
	}

	//Execute next move, decrement counter
	if (m_MoveQueue[m_MoveQueueIndex - 1].wasPlayer)
	{
		ExecuteAttack(m_PlayerParty[m_PlayerActivePkm], m_EnemyParty[m_EnemyActivePkm], m_MoveQueue[m_MoveQueueIndex - 1].move);
	}
	else
	{
		ExecuteAttack(m_EnemyParty[m_EnemyActivePkm], m_PlayerParty[m_PlayerActivePkm], m_MoveQueue[m_MoveQueueIndex - 1].move);
	}
	m_MoveQueueIndex--;

	//Check if won
	if (m_PlayerParty[m_PlayerActivePkm].hp < 0)
	{
		EngineLog("Player Lost!");
	}
	else if (m_EnemyParty[m_EnemyActivePkm].hp < 0)
	{
		EngineLog("Enemy Lost!");
	}
}