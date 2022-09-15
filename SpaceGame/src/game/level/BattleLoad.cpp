#include "game/level/BattleLoad.h"

struct Encounter
{
	int16_t id = -1;
	int32_t weight = 0;
	int8_t lvlLower = 1;
	int8_t lvlUpper = 1;
};

void LoadWildEncounter(Party* party, World::LevelID id)
{
	using namespace rapidxml;

	MtLib::ThreadPool* pool = MtLib::ThreadPool::Fetch();

	//Load encounter table
	std::string path = "res/level/level" + std::to_string((int)id) + "_pok.xml";

	std::shared_ptr<xml_document<>> doc(new xml_document<>());
	std::ifstream* ifs = new std::ifstream(path);
	std::shared_ptr<std::string> tmp(new std::string);
	std::stringstream buffer;
	buffer << ifs->rdbuf();
	ifs->close();
	delete ifs;

	//Buffer and parse
	*tmp.get() = buffer.str();
	doc->parse<0>(&((*tmp.get())[0]));

	//Get encounter root
	xml_node<>* objRoot = doc->first_node()->first_node();

	//Iterate through each pokemon and store encounter rate
	std::vector<Encounter> rates;
	for (xml_node<>* objNode = objRoot->first_node(); objNode; objNode = objNode->next_sibling())
	{
		xml_node<>* idNode = objNode->first_node("ID");
		xml_node<>* weightNode = objNode->first_node("Weight");
		xml_node<>* lvlLNode = objNode->first_node("LvlLower");
		xml_node<>* lvlUNode = objNode->first_node("LvlUpper");
		int16_t id = strtol(idNode->value(), nullptr, 10);
		int16_t weight = strtol(weightNode->value(), nullptr, 10);
		int8_t lvlL = strtol(lvlLNode->value(), nullptr, 10);
		int8_t lvlU = strtol(lvlUNode->value(), nullptr, 10);
		rates.emplace_back(id, weight, lvlL, lvlU);
	}

	//Convert weight to range 1-10000, then offset by previous
	int total = 0;
	for (int i = 0; i < rates.size(); i++)
	{
		total += rates[i].weight;
	}
	for (int i = 0; i < rates.size(); i++)
	{
		float weight = ((float)rates[i].weight / (float)total)*10000;
		rates[i].weight = (int)weight;
		if (i == 0)
		{
			continue;
		}
		rates[i].weight += rates[i - 1].weight;
	}

	//Pick random number 0-9999
	SGRandom::RandomContainer rand; rand.seed(0, 9999);
	float roll = rand.next();

	//Select pokemon in range
	int encounterIndex = 0;
	for (int i = 0; i < rates.size(); i++)
	{
		if (roll < rates[i].weight)
		{
			encounterIndex = i;
			break;
		}
	}

	//Select pokemon level
	int8_t level = 1;
	rand.seed(rates[encounterIndex].lvlLower, rates[encounterIndex].lvlUpper);
	level = (int)rand.next();

	//Create pokemon - TODO make moves dynamic
	PokemonDataBank::loadData(PkmDataType::BASE_STATS);
	PokemonDataBank::loadData(PkmDataType::MOVE_INFO);
	PokemonDataBank::loadData(PkmDataType::POKEMON_TYPES);
	PokemonDataBank::loadData(PkmDataType::SPECIES_INFO);

	party->at(0).id = rates[encounterIndex].id;
	party->at(0).level = level;

	GeneratePokemon(party->at(0).id, party->at(0));
	SetPkmStatsFromLevel(party->at(0));

	party->at(0).nickname = PokemonDataBank::GetPokemonName(party->at(0).id);
	party->at(0).moves[0].id = 7;

	PokemonDataBank::LoadPokemonMoves(party->at(0));
	pool->Run(PokemonDataBank::unloadData, PkmDataType::SPECIES_INFO);
	pool->Run(PokemonDataBank::unloadData, PkmDataType::BASE_STATS);
	pool->Run(PokemonDataBank::unloadData, PkmDataType::MOVE_INFO);
	pool->Run(PokemonDataBank::unloadData, PkmDataType::POKEMON_TYPES);
}