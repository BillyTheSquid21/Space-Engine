#pragma once
#ifndef BATTLE_LOAD_H
#define BATTLE_LOAD_H

#include "rapidxml/rapidxml.hpp"
#include "game/pokemon/Pokemon.h"
#include "game/pokemon/PokemonIO.h"
#include "game/pokemon/PokemonLevel.h"
#include "game/level/World.h"
#include "mtlib/ThreadPool.h"
#include "utility/Random.h"

void LoadWildEncounter(Party* party, World::LevelID id);

#endif