#pragma once
#ifndef MOVE_EFFECTS_H
#define MOVE_EFFECTS_H

#include <stdint.h>
#include <functional>

#include "game/pokemon/Turn.hpp"
#include "game/pokemon/StatStages.hpp"
#include "game/gui/GUI.h"
#include "utility/Random.h"

//Evaluates effects from number value based on the dataset the game uses
//Effects are either named after the effect or the main move that uses it
//Negative numbered effects are not from the dataset
enum class MoveEffectID : int16_t
{
	//Effect
	None = 1, Burn = 5, Paralysis = 7, Sleep = 2, Poison = 3, Freeze = 6,
	Badly_Poisoned = 34, Paralysis_2 = 68,

	//Stages
	Defense_1_Down = 20, Attack_1_Down = 19,

	//Moves
	Dig = 257, Fly = 156
};

//Determines how to queue moves depending on effect
void QueueMovesWithEffects(
	std::function<void(TurnData, int)>& queue, 
	PokemonMove move, 
	Pokemon* origin, 
	Pokemon* target, 
	CurrentStages* originStages, 
	CurrentStages* targetStages,
	PokemonMoveState* originState,
	PokemonMoveState* targetState
);

//Handles effect of moves before turn
void PreMoveEffect(TurnData& turn, SGRandom::RandomContainer& random, GameGUI::TextBuffer& text);
//Handles effect of moves after turn
void PostMoveEffect(TurnData& turn, SGRandom::RandomContainer& random, GameGUI::TextBuffer& text);
//Checks if miss from wrong state
bool CheckPkmStateMiss(PokemonMoveState state);

#endif