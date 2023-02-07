#include "core/Root.h"

#define MAX_RESTARTS 5 //Avoid potential loop

int currentRestarts = 0;
bool shouldRun = true;
bool restart = false;

double SGRoot::FRAMERATE = 0.0;

bool SGRoot::CheckShouldRun()
{
	return shouldRun;
}

void SGRoot::KillProgram()
{
	EngineLog("Shutting program down!");
	shouldRun = false;
}

bool SGRoot::CheckShouldRestart()
{
	if (!restart)
	{
		return false;
	}

	shouldRun = true;
	restart = false;
	return true;
}

void SGRoot::RestartProgram()
{
	if (restart)
	{
		EngineLogFail("Restart");
		return;
	}
	if (currentRestarts >= MAX_RESTARTS)
	{
		restart = false;
		KillProgram();
		EngineLogFail("Restart");
		return;
	}

	KillProgram();
	restart = true;
	currentRestarts++;
	EngineLogOk("Restart");
}

#define COMMAND_COUNT 2

std::string COMMANDS[]
{
	"kill",		//0
	"restart",	//1
};

bool SGRoot::ExecuteCommand(std::string command)
{
	int c = -1;
	for (int i = 0; i < COMMAND_COUNT; i++)
	{
		if (COMMANDS[i] == command)
		{
			c = i;
			break;
		}
	}

	if (c == -1)
	{
		EngineLog("Command not recognised!");
		return false;
	}

	return ExecuteCommand(c);
}

bool SGRoot::ExecuteCommand(int command)
{
	switch (command)
	{
	case 0:
		KillProgram();
		return true;
	case 1:
		RestartProgram();
		return true;
	default:
		EngineLog("Command not recognised!");
		return false;
	}
}