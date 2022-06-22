#pragma once
#ifndef POKEMON_H
#define POKEMON_H

#include <stdint.h>
#include <array>

//Define types
enum class PokemonType : uint8_t
{
	Normal, Null
};

//Define moves
struct PokemonMove
{ 
	//If null move is null
	PokemonType type = PokemonType::Null;
	uint8_t accuracy;
	uint16_t damage;
};

//Define pokemon struct
struct Pokemon
{
	int16_t id = -1; //If id is negative, pokemon is null
	PokemonType primaryType; PokemonType secondaryType;
	//Stats
	int16_t hp = 100;
	int16_t attack = 10;
	int16_t defense = 10;
	int16_t spAttack = 10;
	int16_t spDefense = 10;
	int16_t speed = 10;
	//Moves
	std::array<PokemonMove, 4> moves;
};

typedef std::array<Pokemon, 6> Party;


#endif