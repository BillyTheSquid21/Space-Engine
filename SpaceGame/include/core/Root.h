#pragma once
#ifndef SG_ROOT
#define SG_ROOT

#include "utility/SGUtil.h"
#include "string"
#include "utility/Options.h"

#define COMMAND_COUNT 4
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
	bool Resolution(std::vector<std::string>& args);

	static std::string COMMANDS[]
	{
		"kill",			//0
		"restart",		//1
		"resolution",	//2
		"save-options",	//3
	};

	static enum class COMMAND_CODE : int
	{
		KILL = 0,
		RESTART = 1,
		RESOLUTION = 2,
		SAVE_OPTIONS = 3,
	};

	//Allows converting text to commands
	bool ExecuteCommand(std::vector<std::string>& args);
	bool ExecuteCommand(COMMAND_CODE command, std::vector<std::string>& args);
}

#endif