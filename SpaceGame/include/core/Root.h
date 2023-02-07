#pragma once
#ifndef SG_ROOT
#define SG_ROOT

#include "utility/SGUtil.h"
#include "string"

namespace SGRoot
{
	//Written to in run function, should only be read
	extern double FRAMERATE;

	bool CheckShouldRun();
	void KillProgram();
	bool CheckShouldRestart();
	void RestartProgram();

	//Allows converting text to commands
	bool ExecuteCommand(std::string command);
	bool ExecuteCommand(int command);
}

#endif