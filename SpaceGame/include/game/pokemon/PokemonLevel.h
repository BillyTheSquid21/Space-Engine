#pragma once
#ifndef POKEMON_LEVEL_H
#define POKEMON_LEVEL_H

#include "utility/SGUtil.h"
#include "game/pokemon/Pokemon.h"
#include "game/pokemon/PokemonIO.h"

void SetPkmStatsFromLevel(Pokemon& pokemon);
void LevelUpPkm(Pokemon& pokemon);

#endif