#pragma once
#ifndef GAME_TEXT_H
#define GAME_TEXT_H

#include <string>
#include "game/objects/Script.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"
#include <fstream>
#include <sstream>

//Function that deals with getting the string for text boxes
namespace GameText 
{
	static const std::string pathStart = "res/text/";
	static const std::string lang = "eng/";
	static const std::string ext = ".json";

	std::string FindMessage(MSG_INFO info);
}

#endif