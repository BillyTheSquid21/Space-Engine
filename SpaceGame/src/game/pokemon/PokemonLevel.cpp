#include "game/pokemon/PokemonLevel.h"

//Levelling
static void HPRaiseFormula(Pokemon& pokemon, int16_t& currentStat, int16_t base)
{
	//Numerator
	float iv = 1.0f;
	float ev = 16.0f;
	float numerator = ((2.0f * base) + iv + (ev / 4.0f)) * pokemon.level;
	float newStat = (numerator / 100.0f) + pokemon.level + 10.0f;
	currentStat = (int16_t)newStat;
}

static void StatRaiseFormula(Pokemon& pokemon, int16_t& currentStat, int16_t base)
{
	//Numerator
	float iv = 1.0f;
	float ev = 4.0f;
	float numerator = ((2.0f * base) + iv + (ev / 4.0f)) * pokemon.level;
	float newStat = (numerator / 100.0f) + 5.0f;
	currentStat = (int16_t)newStat;
}

void SetPkmStatsFromLevel(Pokemon& pokemon)
{
	//EV support will be added later
	//Lookup base stats
	PokemonStats baseStats = PokemonDataBank::GetPokemonBaseStats(pokemon.id);
	
	//HP
	HPRaiseFormula(pokemon, pokemon.stats.hp, baseStats.hp);
	pokemon.health = pokemon.stats.hp;
	
	StatRaiseFormula(pokemon, pokemon.stats.attack, baseStats.attack);
	StatRaiseFormula(pokemon, pokemon.stats.defense, baseStats.defense);
	StatRaiseFormula(pokemon, pokemon.stats.spAttack, baseStats.spAttack);
	StatRaiseFormula(pokemon, pokemon.stats.spDefense, baseStats.spDefense);
	StatRaiseFormula(pokemon, pokemon.stats.speed, baseStats.speed);
}

static bool SortByLevel(const rapidjson::Value::ConstValueIterator& lhs, const rapidjson::Value::ConstValueIterator& rhs)
{
	return (*lhs)["level"].GetInt() > (*rhs)["level"].GetInt();
}

void SetPkmMovesFromLevel(Pokemon& pokemon)
{
	using namespace rapidjson;

	//Get moves for pokemon ID
	const Value& moves = PokemonDataBank::GetPermittedMoves(pokemon.id);

	//Find latest level up move pokemon can learn
	std::vector<Value::ConstValueIterator> validIndexes;
	for (Value::ConstValueIterator i = moves.Begin(); i != moves.End(); i++)
	{
		const Value& move = *i;
		//If method isn't level up, continue
		if (move["pokemon_move_method_id"].GetInt() != 1)
		{
			continue;
		}

		//If level learned <= pokemon level, note index
		if (move["level"].GetInt() <= pokemon.level)
		{
			validIndexes.push_back(i);
		}
	}

	//Sort array by level, then take from top value to next 4 or min
	std::sort(std::begin(validIndexes), std::end(validIndexes), SortByLevel);
	int movesIndex = 0;
	for (int i = 0; i < validIndexes.size() && movesIndex < 4; i++)
	{
		pokemon.moves[movesIndex].id = (*(validIndexes[i]))["move_id"].GetInt();
		movesIndex++;
	}
}

void LevelUpPkm(Pokemon& pokemon)
{
	if (pokemon.level >= 100)
	{
		return;
	}
	pokemon.level++;

	//Lookup base stats
	PokemonStats baseStats = PokemonDataBank::GetPokemonBaseStats(pokemon.id);

	//Per stat
	HPRaiseFormula(pokemon, pokemon.stats.hp, baseStats.hp);
	StatRaiseFormula(pokemon, pokemon.stats.attack, baseStats.attack);
	StatRaiseFormula(pokemon, pokemon.stats.defense, baseStats.defense);
	StatRaiseFormula(pokemon, pokemon.stats.spAttack, baseStats.spAttack);
	StatRaiseFormula(pokemon, pokemon.stats.spDefense, baseStats.spDefense);
	StatRaiseFormula(pokemon, pokemon.stats.speed, baseStats.speed);
}