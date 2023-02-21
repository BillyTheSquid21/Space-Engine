#pragma once
#ifndef SG_ROOT
#define SG_ROOT

#include "string"
#include "utility/SGUtil.h"
#include "utility/Options.h"
#include "renderer/RenderSys.h"

#define COMMAND_COUNT 8
#define MAX_RESTARTS 5 //Avoid potential loop

namespace SGRoot
{
	//Written to in run function, should only be read
	extern double FRAMERATE;

	//Polls whether program should continue running
	bool CheckShouldRun();

	//Signals program to finish loop and close down
	void KillProgram();

	//Polls whether program should restart
	bool CheckShouldRestart();

	//Signals program to finish loop and restart (max 5 restarts)
	void RestartProgram();

	//Changes resolution for next restart
	bool Resolution(std::vector<std::string>& args, std::string& output);

	//Changes vsync status for next restart
	void ChangeBool(bool& b, std::vector<std::string>& args, std::string& output);

	//Changes ambient brightness of global lighting
	bool AmbientBright(std::vector<std::string>& args, std::string& output);

	//Lists all commands
	void Help(std::string& output);

	static std::string COMMANDS[]
	{
		"help",			//0
		"kill",			//1
		"restart",		//2
		"resolution",	//3
		"save-options",	//4
		"windowed",		//5
		"vsync",		//6
		"ambient-brightness" //7
	};

	static enum class COMMAND_CODE : int
	{
		HELP = 0,
		KILL = 1,
		RESTART = 2,
		RESOLUTION = 3,
		SAVE_OPTIONS = 4,
		WINDOWED = 5,
		VSYNC = 6,
		AMBIENT_BRIGHT = 7,
	};

	//Allows converting text to commands
	bool ExecuteCommand(std::vector<std::string>& args, std::string& output);
	bool ExecuteCommand(COMMAND_CODE command, std::vector<std::string>& args, std::string& output);
}

#endif