#include "game/pokemon/MoveEffects.h"

static TurnData CreateTurnData(PokemonMove move, Pokemon* origin, Pokemon* target, CurrentStages* originStages, CurrentStages* targetStages,
	PokemonMoveState* originState, PokemonMoveState* targetState)
{
	TurnData data;
	data.move.damage = move.damage;
	data.move.accuracy = move.accuracy;
	data.move.damageType = move.damageType;
	data.move.type = move.type;
	data.move.id = move.id;
	data.move.effectID = move.effect;
	data.move.effectData = move.effectData;
	data.origin = origin; data.target = target;
	data.originStages = originStages; data.targetStages = targetStages;
	data.originState = originState; data.targetState = targetState;
	data.message = origin->nickname + " used " + move.identifier + "!\n";
	return data;
}

//Queue Functions
static void Dig_Effect_Queue(std::function<void(TurnData, int)>& queue, PokemonMove move, Pokemon* origin, Pokemon* target, CurrentStages* originStages, CurrentStages* targetStages,
	PokemonMoveState* originState, PokemonMoveState* targetState)
{
	//Queue for next turn a move that does no damage, does
	//not check accuracy and sets state to digging
	TurnData turn1 = CreateTurnData(move, origin, target, originStages, targetStages, originState, targetState);
	turn1.move.skipAccuracy = true;
	turn1.move.damage = 0;
	turn1.move.effectData = (int8_t)true; //Data says is digging down
	turn1.message = origin->nickname + " dug down!\n";

	//Then queue a move that does the damage of the move and changes state back
	TurnData turn2 = CreateTurnData(move, origin, target, originStages, targetStages, originState, targetState);
	turn2.move.effectData = (int8_t)false; //Data says is now back up

	queue(turn1, 0); queue(turn2, 1);
}

//Pre Effect Functions
static void Dig_Effect_Pre(TurnData& turn, GameGUI::TextBuffer& text)
{
	//Check additional data, if true set state to digging
	//If false, set to normal
	if ((bool)turn.move.effectData)
	{
		*turn.originState = PokemonMoveState::Digging;
		return;
	}
	*turn.originState = PokemonMoveState::Normal;
	return;
}

//Post Effect Functions
static void Burn_Effect_Post(TurnData& turn, RandomContainer& random, GameGUI::TextBuffer& text)
{
	//Roll for whether burns and drop attack
	float roll = random.next();
	if (roll < turn.move.effectData && turn.target->condition == StatusCondition::None)
	{
		turn.target->condition = StatusCondition::Burn;
		turn.targetStages->attackStage = (Stage)((int)turn.targetStages->attackStage - 2);
		text.pushBuffer(turn.target->nickname + " was burned!\n");
	}
}

//Queue
void PostMoveEffect(TurnData& turn, RandomContainer& random, GameGUI::TextBuffer& text)
{
	switch (turn.move.effectID)
	{
	case (int16_t)MoveEffectID::Burn:
		Burn_Effect_Post(turn, random, text);
		return;
	default:
		return;
	}
}

void PreMoveEffect(TurnData& turn, RandomContainer& random, GameGUI::TextBuffer& text)
{
	switch (turn.move.effectID)
	{
	case (int16_t)MoveEffectID::Dig:
		Dig_Effect_Pre(turn, text);
		return;
	default:
		return;
	}
}

void QueueMovesWithEffects(std::function<void(TurnData, int)>& queue, PokemonMove move, Pokemon* origin, Pokemon* target, CurrentStages* originStages, CurrentStages* targetStages,
	PokemonMoveState* originState, PokemonMoveState* targetState)
{
	switch (move.effect)
	{
	case (uint16_t)MoveEffectID::Dig:
		Dig_Effect_Queue(queue, move, origin, target, originStages, targetStages, originState, targetState);
		return;
	default:
		TurnData data = CreateTurnData(move, origin, target, originStages, targetStages, originState, targetState);
		queue(data, 0);
		return;
	}
}

bool CheckPkmStateMiss(PokemonMoveState state)
{
	if (state != PokemonMoveState::Normal)
	{
		return true;
	}
	return false;
}