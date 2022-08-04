#pragma once
#ifndef POKEMON_IO_H
#define POKEMON_IO_H

#include <fstream>
#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

#include <rapidxml/rapidxml.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "mtlib/ThreadPool.h"

#include "game/pokemon/Pokemon.h"
#include "game/items/Items.hpp"
#include "game/utility/XmlWrapper.hpp"

#include "utility/SGUtil.h"

void GeneratePokemon(uint16_t id, Pokemon& pokemon);

//Used as direct keys into array - make sure order added is consistent
enum class PkmDataType
{
	NULL_TYPE = -1, SPECIES_INFO = 0, BASE_STATS = 1, MOVE_INFO = 2, POKEMON_TYPES = 3,
	ITEMS = 4, ITEM_CATEGORIES = 5
};

//Stores any data that will be loaded - TODO make
class PokemonDataBank
{
public:
	static void loadData(PkmDataType type);
	static void unloadData(PkmDataType type);
	static rapidjson::Document& getData(PkmDataType type);
	static bool checkData(PkmDataType type);

	//Pokemon loading functions
	static void LoadPokemonName(uint16_t id, Pokemon& pokemon);
	static void LoadPokemonStats(uint16_t id, Pokemon& pokemon);
	static void LoadPokemonMoves(Pokemon& pokemon);
	static void LoadPokemonType(Pokemon& pokemon);

	//Pokemon data retrieval
	static PokemonStats GetPokemonBaseStats(uint16_t id);
	static std::string GetPokemonName(uint16_t id);

	//Items
	static void InitializeBag(PlayerBag& bag);

private:

	static void loadJson(std::string path, PkmDataType type);
	static std::string getPath(PkmDataType type);

	//Stores the name of each file and the type to be loaded for
	static const std::string filePathStart;
	static const size_t fileCount = 6;
	static const std::string filenames[fileCount];

	//Storage
	struct PkmData
	{
		rapidjson::Document document;
		PkmDataType type;
	};

	static std::vector<PkmData> data;
	static std::mutex mutex;
};

#endif
