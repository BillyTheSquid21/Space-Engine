#include "game/pokemon/PokemonIO.h"

//Function for converting short ID's to string
//Useful for avoiding constantly using std::to_string to get similar short numbers 
template <int places>
static inline void ID_To_String(char(&id)[places + 1], uint16_t index)
{
	id[places] = '\0';

	uint16_t base = 10;
	uint16_t result = index;

	for (int i = places - 1; i >= 0; i--)
	{
		id[i] = (result % base) + 48;
		result = result / base;
	}

	int times = 1; int size;
	while (id[0] == '0' && times < places)
	{
		size = 5 - times;
		memmove(&id[0], &id[1], size);
		times++;
	}
}

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
	"pokemon/pokemon.json",
	"pokemon/pokemon_stats.json",
	"pokemon/moves.json",
	"pokemon/pokemon_types.json",
	"items/items.json",
	"items/item_categories.json"
};
std::vector<PokemonDataBank::PkmData> PokemonDataBank::data;
const std::string PokemonDataBank::filePathStart = "res/data/";
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
	std::lock_guard lock(mutex);
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
	std::lock_guard lock(mutex);
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
	std::lock_guard lock(mutex);
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
	std::lock_guard lock(mutex);
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
	std::lock_guard lock(mutex);
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
	std::lock_guard lock(mutex);
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
	std::lock_guard lock(mutex);
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
		move.accuracy = 100;
	}
	else
	{
		move.accuracy = stats["accuracy"].GetInt();
	}
	move.type = (PokemonType)stats["type_id"].GetInt();
	move.effect = stats["effect_id"].GetInt();
	if (!stats["effect_chance"].IsNull())
	{
		move.effectData = stats["effect_chance"].GetInt();
	}
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
	std::lock_guard lock(mutex);
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
	std::lock_guard lock(mutex);
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

//Items
static bool SortByPocket(const ItemSlot& lhs, const ItemSlot& rhs)
{
	//If is a tm, order alphabetically
	if (lhs.item.pocketId == 4 && rhs.item.pocketId == 4)
	{
		return lhs.item.name < rhs.item.name;
	}
	return lhs.item.pocketId > rhs.item.pocketId;
}

void PokemonDataBank::InitializeBag(PlayerBag& bag)
{
	using namespace rapidjson;
	if (!PokemonDataBank::checkData(PkmDataType::ITEMS))
	{
		EngineLog("Items not loaded!");
		return;
	}
	if (!PokemonDataBank::checkData(PkmDataType::ITEM_CATEGORIES))
	{
		EngineLog("Item categories not loaded!");
		return;
	}

	rapidjson::Document& itemDoc = PokemonDataBank::getData(PkmDataType::ITEMS);
	rapidjson::Document& categoryDoc = PokemonDataBank::getData(PkmDataType::ITEM_CATEGORIES);
	std::lock_guard lock(mutex);

	//Go through each entry in the item json file
	char id[5] = { '1', '\0' };
	for (uint32_t i = 0; i < MAX_ITEM_TYPES; i++)
	{
		ID_To_String<4>(id, i);
		if (!itemDoc.HasMember(id))
		{
			continue;
		}
		const Value& itemData = itemDoc[id];
		int categoryID = itemData["category_id"].GetInt();
		ID_To_String<4>(id, categoryID);

		const Value& categoryData = categoryDoc[id];
		Item item = {i, categoryData["pocket_id"].GetInt(), categoryID, itemData["identifier"].GetString() };
		bag.items[i].item = item; //bag.items[i].count = 1; //Temp to show all
	}

	//Sort array in order of pocket ID
	std::sort(bag.items.begin(), bag.items.end(), SortByPocket);
}