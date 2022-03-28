#pragma once
#ifndef GAME_MESSAGE_H
#define GAME_MESSAGE_H

//Message enum - it is up to object implementations to decide what a message means
enum class Message
{
	//CGE is shorthand for change
	NO_OP, ACTIVATE, DEACTIVATE, KILL, CGE_BOOL, BUSY,
};

#endif
