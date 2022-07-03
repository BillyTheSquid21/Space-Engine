#pragma once
#ifndef POKEMON_IO_H
#define POKEMON_IO_H

#include <fstream>
#include <string>
#include <sstream>
#include <memory>

#include "rapidxml/rapidxml.hpp"
#include "game/pokemon/Pokemon.h"

Pokemon GeneratePokemon(uint16_t id);

class PokemonDataBank
{
public:

private:
	static std::vector<XML_Doc_Wrapper> databank;
};

#endif
