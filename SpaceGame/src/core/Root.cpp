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

bool SGRoot::Resolution(std::vector<std::string>& args, std::string& output)
{
	//Ensure 3 arguments (command, w, h)
	if (args.size() != 3)
	{
		output = "invalid parameters!";
		return false;
	}

	//Try to convert to ints, if valid string
	try
	{
		int w = std::stoi(args[1], nullptr, 10);
		int h = std::stoi(args[2], nullptr, 10);

		if (w > MAX_WIDTH || h > MAX_HEIGHT || w < MIN_WIDTH || h < MIN_HEIGHT)
		{
			output = "invalid dimensions!";
			return false;
		}

		SGOptions::WIDTH = w;
		SGOptions::HEIGHT = h;
		output = "set resolution: " + args[1] + " " + args[2];
		return true;
	}
	catch (std::invalid_argument const& ex)
	{
		output = "invalid argument!" + std::string(ex.what());
		return false;
	}
	catch (std::out_of_range const& ex)
	{
		output = "out of range!" + std::string(ex.what());
		return false;
	}
}

void SGRoot::ChangeBool(bool& b, std::vector<std::string>& args, std::string& output)
{
	if (args.size() != 2)
	{
		output = "invalid parameters!";
		return;
	}

	if (args[1] == "enable" || args[1] == "true")
	{
		b = true;
		output = "enabled";
	}
	else if (args[1] == "disable" || args[1] == "false")
	{
		b = false;
		output = "disabled";
	}
	else
	{
		output = "invalid parameter!";
	}
}

void SGRoot::Help(std::string& output)
{
	output += COMMANDS[0] + "\n";
	for (int i = 1; i < COMMAND_COUNT; i++)
	{
		output += " " + COMMANDS[i] + "\n";
	}
}

bool SGRoot::ExecuteCommand(std::vector<std::string>& args, std::string& output)
{
	if (args.size() == 0)
	{
		return false;
	}

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
		output = "command not recognised: " + args[0];
		return false;
	}

	return ExecuteCommand((COMMAND_CODE)c, args, output);
}

bool SGRoot::ExecuteCommand(COMMAND_CODE command, std::vector<std::string>& args, std::string& output)
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
		Resolution(args, output);
		return true;
	case COMMAND_CODE::SAVE_OPTIONS:
		SGOptions::SaveOptions();
		output = "saving options";
		return true;
	case COMMAND_CODE::WINDOWED:
		ChangeBool(SGOptions::WINDOWED, args, output);
		return true;
	case COMMAND_CODE::VSYNC:
		ChangeBool(SGOptions::VSYNC_ENABLED, args, output);
		return true;
	case COMMAND_CODE::HELP:
		Help(output);
		return true;
	default:
		EngineLog("Command not recognised!");
		return false;
	}
}