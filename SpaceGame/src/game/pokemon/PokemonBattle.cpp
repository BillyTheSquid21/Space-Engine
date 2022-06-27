#include "game/pokemon/PokemonBattle.h"

RandomContainer PokemonBattle::random;

uint8_t LookupTypeMultiplier(PokemonType attacking, PokemonType defending)
{
	return TypeLookup[(int)attacking][(int)defending];
}

void ExecuteAttack(Pokemon& attacker, Pokemon& target, PokemonMove move)
{
	//Damage

	//Roll if misses
	float roll = PokemonBattle::random.next();
	if (roll < move.damageAcc * 100)
	{
		EngineLog("HP Before: ", target.health);
		int16_t damage = CalculateDamage(attacker, target, move);
		target.health -= damage;
		EngineLog("HP After: ", target.health);
		EngineLog("DAMAGE: ", damage);
	}
	else
	{
		EngineLog("MISSED!");
	}

	//Status
	if (move.status == StatusCondition::None)
	{
		return;
	}

	roll = PokemonBattle::random.next();
	if (roll < move.statusAcc * 100)
	{
		EngineLog("Status condition: ", (int)move.status);
		target.condition = move.status;
		if (move.status == StatusCondition::Sleep)
		{
			target.storage = 2;
		}
	}
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
	float type = 1.0f;
	//Use primary type of attacker
	float primaryMod = (float)LookupTypeMultiplier(move.type, target.primaryType) / 2.0f;
	float secondaryMod = 1.0f;
	if (target.secondaryType != PokemonType::Null)
	{
		secondaryMod = (float)LookupTypeMultiplier(move.type, target.secondaryType) / 2.0f;
	}
	type *= primaryMod * secondaryMod;

	float level = ((2 * (float)attacker.level) / 5) + 2;
	float attackOverDefence = (float)attacker.attack / (float)target.defense;
	float damageOut = ((level * (float)move.damage * attackOverDefence) / 50.0f) + 2.0f;

	//Now multiply premultiplier by additional effects - easy to add
	damageOut *= critical * randomRoll * stab * type;

	//Cast to int
	return (int16_t)damageOut;
}

void ProcessEndTurnStatus(Pokemon& pokemon)
{
	uint16_t damage;
	switch (pokemon.condition)
	{
	case StatusCondition::None:
		return;
	case StatusCondition::Poison:
		//Calc damage taken
		damage = pokemon.hp / 16;
		pokemon.health -= damage;
		EngineLog("Was damaged by poison!");
		return;
	case StatusCondition::Burn:
		//Calc damage taken
		damage = pokemon.hp / 16;
		pokemon.health -= damage;
		EngineLog("Was hurt by burn!");
		return;
	default:
		return;
	}
}

bool ProcessStatus(Pokemon& pokemon)
{
	bool skipTurn = false;
	float roll = 0.0f;
	switch (pokemon.condition)
	{
	case StatusCondition::None:
		return false;
	case StatusCondition::Sleep:
		//Check if sleep timer (kept in storage) has hit zero - if so sleep ends
		if (pokemon.storage <= 0)
		{
			EngineLog("Woke up!");
			pokemon.condition = StatusCondition::None;
			pokemon.storage = 0;
			return false;
		}
		//Otherwise roll for a chance to end - is 33.3% so roll 3,333/10,000
		roll = PokemonBattle::random.next();
		if (roll <= 3333.0f)
		{
			EngineLog("Woke up!");
			pokemon.condition = StatusCondition::None;
			pokemon.storage = 0;
			return false;
		}
		EngineLog("Is asleep...");
		pokemon.storage--;
		return true;
	case StatusCondition::Paralysis:
		//Roll for a chance to miss turn - is 25%
		roll = PokemonBattle::random.next();
		if (roll < 7500.0f)
		{
			return false;
		}
		EngineLog("Is paralyzed! And isn't able to move!");
		return true;
	default:
		return false;
	}
}

void PokemonBattle::nextMove()
{
	//Execute top move
	Team teamAttacking = m_MoveQueue[m_MoveQueueIndex - 1].team;
	if (teamAttacking == Team::A)
	{
		//First check if any status effect change outcome
		if (!ProcessStatus(m_PartyA[m_ActivePkmA]))
		{
			ExecuteAttack(m_PartyA[m_ActivePkmA], m_PartyB[m_ActivePkmB], m_MoveQueue[m_MoveQueueIndex - 1].move);
		}
	}
	else if (teamAttacking == Team::B)
	{
		//First check if any status effect change outcome
		if (!ProcessStatus(m_PartyB[m_ActivePkmB]))
		{
			ExecuteAttack(m_PartyB[m_ActivePkmB], m_PartyA[m_ActivePkmA], m_MoveQueue[m_MoveQueueIndex - 1].move);
		}
	}
	//Check if anyone is dead
	if (m_PartyA[m_ActivePkmA].health < 0)
	{
		EngineLog("Player Lost!");
		return;
	}
	else if (m_PartyB[m_ActivePkmB].health < 0)
	{
		EngineLog("Enemy Lost!");
		return;
	}
	m_MoveQueueIndex--;
}

void PokemonBattle::run(bool progress)
{
	if (!progress)
	{
		return;
	}
	//Check if won
	if (m_PartyA[m_ActivePkmA].health <= 0)
	{
		EngineLog("Player Lost!");
		return;
	}
	else if (m_PartyB[m_ActivePkmB].health <= 0)
	{
		EngineLog("Enemy Lost!");
		return;
	}

	EngineLog("BATTLE: ", m_MoveQueueIndex);
	EngineLog("Player HP: ", m_PartyA[m_ActivePkmA].health);
	EngineLog("Enemy HP: ", m_PartyB[m_ActivePkmB].health);

	//Await player input, add to queue
	m_MoveQueue[m_MoveQueueIndex] = { m_PartyA[m_ActivePkmA].moves[0], Team::A };
	m_MoveQueueIndex++;
	
	//Await enemy input, add to queue
	m_MoveQueue[m_MoveQueueIndex] = { m_PartyB[m_ActivePkmB].moves[0], Team::B };
	m_MoveQueueIndex++;

	this->nextMove();
	//Check if won
	if (m_PartyA[m_ActivePkmA].health <= 0)
	{
		EngineLog("Player Lost!");
		return;
	}
	else if (m_PartyB[m_ActivePkmB].health <= 0)
	{
		EngineLog("Enemy Lost!");
		return;
	}
	this->nextMove();
	//Check if won
	if (m_PartyA[m_ActivePkmA].health <= 0)
	{
		EngineLog("Player Lost!");
		return;
	}
	else if (m_PartyB[m_ActivePkmB].health <= 0)
	{
		EngineLog("Enemy Lost!");
		return;
	}

	//Process end turn status
	ProcessEndTurnStatus(m_PartyA[m_ActivePkmA]);
	ProcessEndTurnStatus(m_PartyB[m_ActivePkmB]);
}