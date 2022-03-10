#include "game/utility/GameText.h"

static std::string test = "Hi. This is a line of text!";

std::string& FindMessage(MSG_INFO info)
{
	//For now return simple message
	return test;
}