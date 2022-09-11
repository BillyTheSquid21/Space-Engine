#pragma once
#ifndef GAME_OPTIONS_HPP
#define GAME_OPTIONS_HPP

#include "fmod.h"
#include "core/Sound.h"
#include "fstream"
#include "ostream"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/writer.h"

struct Options
{
	//Audio Settings
	static FMOD::ChannelGroup* effects;
	static FMOD::ChannelGroup* music;

	//Graphic Settings
	static int grassDensity;

	static void load();
	static void write();
};

#endif