#pragma once
#ifndef MOVE_EFFECTS_H
#define MOVE_EFFECTS_H

//Evaluates effects from number value based on the dataset the game uses
//Effects are either named after the effect or the main move that uses it
enum class MoveEffectID
{
	//Effect
	None = 1, Burn = 5, Paralysis = 7, Sleep = 2, Poison = 3, Freeze = 6, 
	Badly_Poisoned = 34,

	//Stages
	Defense_1_Down = 20, Attack_1_Down = 19,
	
	//Moves
	Thunder_Wave = 68, Dig = 257, 
};

#endif