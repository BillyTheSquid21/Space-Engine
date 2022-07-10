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