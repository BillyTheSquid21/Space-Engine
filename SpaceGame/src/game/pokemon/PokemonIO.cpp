#include "game/pokemon/PokemonIO.h"

static void AddStat(Pokemon& pkm, int stat, int value)
{
	switch (stat)
	{
	case 0:
		pkm.stats.hp = value;
		pkm.health = value;
		break;
	case 1:
		pkm.stats.attack = value;
		break;
	case 2:
		pkm.stats.defense = value;
		break;
	case 3:
		pkm.stats.spAttack = value;
		break;
	case 4:
		pkm.stats.spDefense = value;
		break;
	case 5:
		pkm.stats.speed = value;
		break;
	default:
		break;
	}
}

static void AddStat(PokemonStats& stats, int stat, int value)
{
	switch (stat)
	{
	case 0:
		stats.hp = value;
		break;
	case 1:
		stats.attack = value;
		break;
	case 2:
		stats.defense = value;
		break;
	case 3:
		stats.spAttack = value;
		break;
	case 4:
		stats.spDefense = value;
		break;
	case 5:
		stats.speed = value;
		break;
	default:
		break;
	}
}

//Data Bank
const std::string PokemonDataBank::filenames[fileCount] =
{
	"pokemon.json",
	"pokemon_stats.json",
	"moves.json",
	"pokemon_types.json"
};
std::vector<PokemonDataBank::PkmData> PokemonDataBank::data;
const std::string PokemonDataBank::filePathStart = "res/pokemon/";
std::mutex PokemonDataBank::mutex;

std::string PokemonDataBank::getPath(PkmDataType type)
{
	if ((int)type >= fileCount || type == PkmDataType::NULL_TYPE)
	{
		EngineLog("Error retrieving pokemon data path!");
		return filenames[0];
	}
	return filenames[(int)type];
}

void PokemonDataBank::loadJson(std::string path, PkmDataType type)
{
	//Parse and store
	using namespace rapidjson;
	std::ifstream ifs((filePathStart + path).c_str());
	rapidjson::IStreamWrapper isw(ifs);

	//Emplace data
	data.emplace_back();
	data.back().document.ParseStream(isw);

	ifs.close();

	//Set type
	data.back().type = type;
}

void PokemonDataBank::loadData(PkmDataType type)
{
	if (checkData(type))
	{
		return;
	}
	std::lock_guard lock(mutex);
	loadJson(getPath(type), type);
}

void PokemonDataBank::unloadData(PkmDataType type)
{
	std::lock_guard lock(mutex);
	for (int i = 0; i < data.size(); i++)
	{
		if (data[i].type == type)
		{
			data.erase(data.begin() + i);
			return;
		}
	}
	EngineLog("Data wasn't found!");
}

bool PokemonDataBank::checkData(PkmDataType type)
{
	for (int i = 0; i < data.size(); i++)
	{
		if (data[i].type == type)
		{
			return true;
		}
	}
	return false;
}

rapidjson::Document& PokemonDataBank::getData(PkmDataType type)
{
	for (int i = 0; i < data.size(); i++)
	{
		if (data[i].type == type)
		{
			return data[i].document;
		}
	}
	EngineLog("Data not loaded, try checking exists first!");
	return data[0].document;
}

//Load pokemon
void PokemonDataBank::LoadPokemonName(uint16_t id, Pokemon& pokemon)
{
	using namespace rapidjson;

	if (!PokemonDataBank::checkData(PkmDataType::SPECIES_INFO))
	{
		EngineLog("Species info not loaded!");
		return;
	}

	rapidjson::Document& doc = PokemonDataBank::getData(PkmDataType::SPECIES_INFO);
	std::string idString = std::to_string(id);
	if (!doc.HasMember(idString.c_str()))
	{
		return;
	}

	const Value& stats = doc[idString.c_str()];
	pokemon.nickname = stats["identifier"].GetString();
}

void PokemonDataBank::LoadPokemonStats(uint16_t id, Pokemon& pokemon)
{
	using namespace rapidjson;
	if (!PokemonDataBank::checkData(PkmDataType::BASE_STATS))
	{
		EngineLog("Base stats not loaded!");
		return;
	}

	rapidjson::Document& doc = PokemonDataBank::getData(PkmDataType::BASE_STATS);
	std::string idString = std::to_string(id);
	if (!doc.HasMember(idString.c_str()))
	{
		return;
	}

	const Value& stats = doc[idString.c_str()];
	for (SizeType i = 0; i < stats.Size(); ++i)
	{
		if (stats[i].HasMember("base_stat")) {
			AddStat(pokemon, i, stats[i]["base_stat"].GetInt());
		}
	}
}

PokemonStats PokemonDataBank::GetPokemonBaseStats(uint16_t id)
{
	using namespace rapidjson;
	PokemonStats baseStats = {1,1,1,1,1,1};
	if (!PokemonDataBank::checkData(PkmDataType::BASE_STATS))
	{
		EngineLog("Base stats not loaded!");
		return baseStats;
	}

	rapidjson::Document& doc = PokemonDataBank::getData(PkmDataType::BASE_STATS);
	std::string idString = std::to_string(id);
	
	if (!doc.HasMember(idString.c_str()))
	{
		return baseStats;
	}

	const Value& stats = doc[idString.c_str()];
	for (SizeType i = 0; i < stats.Size(); ++i)
	{
		if (stats[i].HasMember("base_stat")) {
			AddStat(baseStats, i, stats[i]["base_stat"].GetInt());
		}
	}
	return baseStats;
}

std::string PokemonDataBank::GetPokemonName(uint16_t id)
{
	using namespace rapidjson;
	std::string name = "bulbasaur";
	if (!PokemonDataBank::checkData(PkmDataType::SPECIES_INFO))
	{
		EngineLog("Species info not loaded!");
		return name;
	}

	rapidjson::Document& doc = PokemonDataBank::getData(PkmDataType::SPECIES_INFO);
	std::string idString = std::to_string(id);
	if (!doc.HasMember(idString.c_str()))
	{
		return name;
	}

	const Value& stats = doc[idString.c_str()];
	name = stats["identifier"].GetString();
	return name;
}

inline static void LoadMoveData(Pokemon& pokemon, rapidjson::Document& doc, std::string idString, unsigned int moveIndex)
{
	using namespace rapidjson;
	const Value& stats = doc[idString.c_str()];
	PokemonMove& move = pokemon.moves[moveIndex];
	move.identifier = stats["identifier"].GetString();
	move.damageType = (DamageType)stats["damage_class_id"].GetInt();
	if (stats["power"].IsNull())
	{
		move.damage = 0;
	}
	else
	{
		move.damage = stats["power"].GetInt();
	}
	if (stats["accuracy"].IsNull())
	{
		move.damageAcc = 100;
	}
	else
	{
		move.damageAcc = stats["accuracy"].GetInt();
	}
	move.type = (PokemonType)stats["type_id"].GetInt();

}

void PokemonDataBank::LoadPokemonMoves(Pokemon& pokemon)
{
	using namespace rapidjson;

	if (!PokemonDataBank::checkData(PkmDataType::MOVE_INFO))
	{
		EngineLog("Move info not loaded!");
		return;
	}

	rapidjson::Document& doc = PokemonDataBank::getData(PkmDataType::MOVE_INFO);
	std::string idString;

	//Moves in slot order
	idString = std::to_string(pokemon.moves[0].id);
	if (pokemon.moves[0].id > 0 && doc.HasMember(idString.c_str()))
	{
		LoadMoveData(pokemon, doc, idString,0);
	}
	idString = std::to_string(pokemon.moves[1].id);
	if (pokemon.moves[1].id > 0 && doc.HasMember(idString.c_str()))
	{
		LoadMoveData(pokemon, doc, idString,1);
	}
	idString = std::to_string(pokemon.moves[2].id);
	if (pokemon.moves[2].id > 0 && doc.HasMember(idString.c_str()))
	{
		LoadMoveData(pokemon, doc, idString,2);
	}
	idString = std::to_string(pokemon.moves[3].id);
	if (pokemon.moves[3].id > 0 && doc.HasMember(idString.c_str()))
	{
		LoadMoveData(pokemon, doc, idString,3);
	}
}

void PokemonDataBank::LoadPokemonType(Pokemon& pokemon)
{
	using namespace rapidjson;
	if (!PokemonDataBank::checkData(PkmDataType::POKEMON_TYPES))
	{
		EngineLog("Types not loaded!");
		return;
	}

	rapidjson::Document& doc = PokemonDataBank::getData(PkmDataType::POKEMON_TYPES);
	std::string idString = std::to_string(pokemon.id);
	if (!doc.HasMember(idString.c_str()))
	{
		return;
	}

	const Value& stats = doc[idString.c_str()];
	for (SizeType i = 0; i < stats.Size(); ++i)
	{
		int slot = stats[i]["slot"].GetInt();
		switch (slot)
		{
		case 1:
			pokemon.primaryType = (PokemonType)stats[i]["type_id"].GetInt();
			break;
		case 2:
			pokemon.secondaryType = (PokemonType)stats[i]["type_id"].GetInt();
			break;
		default:
			break;
		}
	}
}

void GeneratePokemon(uint16_t id, Pokemon& pokemon)
{
	PokemonDataBank::LoadPokemonName(id, pokemon);
	PokemonDataBank::LoadPokemonMoves(pokemon);
	PokemonDataBank::LoadPokemonType(pokemon);
}