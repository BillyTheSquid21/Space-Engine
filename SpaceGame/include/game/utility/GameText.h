#pragma once
#ifndef GAME_TEXT_H
#define GAME_TEXT_H

#include <string>
#include "game/objects/Script.hpp"

//Function that deals with getting the string for text boxes
std::string& FindMessage(MSG_INFO info);

#endif