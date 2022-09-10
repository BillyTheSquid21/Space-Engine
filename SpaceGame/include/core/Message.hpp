#pragma once
#ifndef GAME_MESSAGE_H
#define GAME_MESSAGE_H

#include <stdint.h>

namespace SGObject
{
	/**
	* Message enum that maps onto the uint32_t type
	* Provides some basic messages for use in object communication
	* As are ints, can interpret however desired, and can create own codes if desired
	*/
	enum class Message : uint32_t
	{
		//CGE is shorthand for change
		NO_OP, ACTIVATE, DEACTIVATE, KILL, BUSY,
	};
}

#endif
