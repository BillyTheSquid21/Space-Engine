#pragma once
#ifndef GAME_MESSAGE_H
#define GAME_MESSAGE_H

#include <stdint.h>

//Message enum - it is up to object implementations to decide what a message means
//Gives a few base messages that objects understand by default
enum class Message : uint32_t
{
	//CGE is shorthand for change
	NO_OP, ACTIVATE, DEACTIVATE, KILL, CGE_BOOL, BUSY,
};

#endif
