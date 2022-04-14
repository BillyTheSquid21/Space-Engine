#include "game/utility/GameText.h"

std::string GameText::FindMessage(MSG_INFO info)
{
	//Get path of desired file
	using namespace GameText;
	std::string textList = std::to_string(info.listID);
	std::string path = pathStart + lang + textList + ext;
	std::ifstream ifs(path);
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document doc;
	doc.ParseStream(isw);
	ifs.close();

	//Get line is on
	std::string lineID = std::to_string(info.textEntry);
	if (!doc.HasMember(lineID.c_str())) //Return blank if not present
	{
		return " ";
	}

	std::string message = doc[lineID.c_str()].GetString();

	return message;
}