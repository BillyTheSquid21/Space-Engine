#include "core/Root.h"

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

bool SGRoot::Resolution(std::vector<std::string>& args)
{
	//Ensure 3 arguments (command, w, h)
	if (args.size() != 3)
	{
		EngineLog("Invalid parameters!");
		return false;
	}

	//Try to convert to ints, if valid string
	try
	{
		int w = std::stoi(args[1], nullptr, 10);
		int h = std::stoi(args[2], nullptr, 10);

		if (w > MAX_WIDTH || h > MAX_HEIGHT || w < MIN_WIDTH || h < MIN_HEIGHT)
		{
			EngineLog("Invalid dimensions!");
			return false;
		}

		SGOptions::WIDTH = w;
		SGOptions::HEIGHT = h;
		EngineLogOk("Set resolution: ", w, " ", h);
		return true;
	}
	catch (std::invalid_argument const& ex)
	{
		EngineLog("Invalid argument!", ex.what());
		return false;
	}
	catch (std::out_of_range const& ex)
	{
		EngineLog("Out of range!", ex.what());
		return false;
	}
}

bool SGRoot::ExecuteCommand(std::vector<std::string>& args)
{
	int c = -1;
	for (int i = 0; i < COMMAND_COUNT; i++)
	{
		if (COMMANDS[i] == args[0])
		{
			c = i;
			break;
		}
	}

	if (c == -1)
	{
		EngineLog(args[0]);
		EngineLog("Command not recognised!");
		return false;
	}

	return ExecuteCommand((COMMAND_CODE)c, args);
}

bool SGRoot::ExecuteCommand(COMMAND_CODE command, std::vector<std::string>& args)
{
	switch (command)
	{
	case COMMAND_CODE::KILL:
		KillProgram();
		return true;
	case COMMAND_CODE::RESTART:
		RestartProgram();
		return true;
	case COMMAND_CODE::RESOLUTION:
		Resolution(args);
		return true;
	case COMMAND_CODE::SAVE_OPTIONS:
		SGOptions::SaveOptions();
		return true;
	default:
		EngineLog("Command not recognised!");
		return false;
	}
}