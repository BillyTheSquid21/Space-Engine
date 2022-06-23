#include "game/pokemon/PokemonBattle.h"

RandomContainer PokemonBattle::random;

uint8_t LookupTypeMultiplier(PokemonType attacking, PokemonType defending)
{
	return TypeLookup[(int)attacking][(int)defending];
}

void ExecuteAttack(Pokemon& attacker, Pokemon& target, PokemonMove move)
{
	EngineLog("HP Before: ", target.hp);
	int16_t damage = CalculateDamage(attacker, target, move);
	target.hp -= damage;
	EngineLog("HP After: ", target.hp);
	EngineLog("DAMAGE: ", damage);
}

int16_t CalculateDamage(Pokemon& attacker, Pokemon& target, PokemonMove move)
{
	//Calc whether critical hit - 6.25%
	float critical = 1;
	float critRoll = PokemonBattle::random.next();
	if (critRoll < 625.0f)
	{
		critical = 2;
		EngineLog("Its a critical hit!");
	}

	//Calc random factor and clamp between 85 and 100
	float randomRoll = (85.0f + (float)PokemonBattle::random.next() * (15.0f/(float)BATTLE_PROBABILITY_MAX)) / 100.0f;

	//Calc stab
	float stab = 1.0f;
	if (attacker.primaryType == move.type || attacker.secondaryType == move.type)
	{
		stab = 1.5f;
	}

	//Calc type effectiveness

	float level = ((2 * (float)attacker.level) / 5) + 2;
	float attackOverDefence = (float)attacker.attack / (float)target.defense;
	float damageOut = ((level * (float)move.damage * attackOverDefence) / 50.0f) + 2.0f;

	//Now multiply premultiplier by additional effects - easy to add
	damageOut *= critical * randomRoll * stab;

	//Cast to int
	return (int16_t)damageOut;
}

void PokemonBattle::run()
{
	//Check if won
	if (m_PlayerParty[m_PlayerActivePkm].hp < 0)
	{
		EngineLog("Player Lost!");
		return;
	}
	else if (m_EnemyParty[m_EnemyActivePkm].hp < 0)
	{
		EngineLog("Enemy Lost!");
		return;
	}

	EngineLog("BATTLE: ", m_MoveQueueIndex);
	EngineLog("Player HP: ", m_PlayerParty[m_PlayerActivePkm].hp);
	EngineLog("Enemy HP: ", m_EnemyParty[m_EnemyActivePkm].hp);

	//Await player input, add to queue
	std::string move1;
	std::cin >> move1;
	if (move1 == "1")
	{
		m_MoveQueue[m_MoveQueueIndex] = { m_PlayerParty[m_PlayerActivePkm].moves[0], true };
	}
	m_MoveQueueIndex++;
	
	//Await enemy input, add to queue
	m_MoveQueue[m_MoveQueueIndex] = { m_EnemyParty[m_EnemyActivePkm].moves[0], false };
	m_MoveQueueIndex++;

	//Execute top move
	if (m_MoveQueue[m_MoveQueueIndex - 1].wasPlayer)
	{
		ExecuteAttack(m_PlayerParty[m_PlayerActivePkm], m_EnemyParty[m_EnemyActivePkm], m_MoveQueue[m_MoveQueueIndex - 1].move);
		if (m_EnemyParty[m_EnemyActivePkm].hp < 0)
		{
			EngineLog("Enemy Lost!");
			return;
		}
	}
	else
	{
		ExecuteAttack(m_EnemyParty[m_EnemyActivePkm], m_PlayerParty[m_PlayerActivePkm], m_MoveQueue[m_MoveQueueIndex - 1].move);
		if (m_PlayerParty[m_PlayerActivePkm].hp < 0)
		{
			EngineLog("Player Lost!");
			return;
		}
	}
	m_MoveQueueIndex--;

	//Execute next move, decrement counter
	if (m_MoveQueue[m_MoveQueueIndex - 1].wasPlayer)
	{
		ExecuteAttack(m_PlayerParty[m_PlayerActivePkm], m_EnemyParty[m_EnemyActivePkm], m_MoveQueue[m_MoveQueueIndex - 1].move);
		if (m_EnemyParty[m_EnemyActivePkm].hp < 0)
		{
			EngineLog("Enemy Lost!");
			return;
		}
	}
	else
	{
		ExecuteAttack(m_EnemyParty[m_EnemyActivePkm], m_PlayerParty[m_PlayerActivePkm], m_MoveQueue[m_MoveQueueIndex - 1].move);
		if (m_PlayerParty[m_PlayerActivePkm].hp < 0)
		{
			EngineLog("Player Lost!");
			return;
		}
	}
	m_MoveQueueIndex--;
}