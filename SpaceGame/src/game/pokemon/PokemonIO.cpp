#include "game/pokemon/PokemonIO.h"

Pokemon GeneratePokemon(uint16_t id)
{
	//Get name
	using namespace rapidxml;
	std::string path = "";
	path = "res/pokemon/pokemon.xml";

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

	//Get root
	xml_node<>* objRoot = doc->first_node();

	Pokemon pokemon;

	//Get pokemon name as test
	for (xml_node<>* objNode = objRoot->first_node(); objNode; objNode = objNode->next_sibling())
	{
		int objID = std::strtoul(objNode->first_node()->value(), nullptr, 10);
		if (objID != id)
		{
			continue;
		}
		xml_node<>* nameNode = objNode->first_node("identifier");
		pokemon.nickname = nameNode->value();
	}
	return pokemon;
}