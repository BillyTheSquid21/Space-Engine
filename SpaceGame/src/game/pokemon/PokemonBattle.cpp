#include "game/pokemon/PokemonBattle.h"

RandomContainer PokemonBattle::random;
RandomContainer MoveQueue::random;

bool SortBySpeed(const MoveQueue::MoveTurn::Combined& lhs, const MoveQueue::MoveTurn::Combined& rhs)
{
	return lhs.origin->speed < rhs.origin->speed;
}

uint8_t LookupTypeMultiplier(PokemonType attacking, PokemonType defending)
{
	return TypeLookup[(int)attacking][(int)defending];
}

float LookupStageMultiplier(Stage stage)
{
	return StatStageLookup[(int)stage];
}

void ExecuteAttack(Pokemon& attacker, Pokemon& target, PokemonMove move, CurrentStages& attackerStage, CurrentStages& defenderStage)
{
	BattleTextBuffer::clearText();
	BattleTextBuffer::pushText(attacker.nickname + " " + "used " + std::to_string(move.id));
	PokemonBattle::awaitInput();

	//Damage

	//Roll if misses
	float roll = PokemonBattle::random.next();
	if (roll < move.damageAcc * 100)
	{
		int16_t damage = CalculateDamage(attacker, target, move, attackerStage, defenderStage);
		target.health -= damage;

		//Check if was SE
		//Calc type effectiveness
		float type = 1.0f;
		//Use primary type of attacker
		float primaryMod = (float)LookupTypeMultiplier(move.type, target.primaryType) / 2.0f;
		float secondaryMod = 1.0f;
		if (target.secondaryType != PokemonType::Null)
		{
			secondaryMod = (float)LookupTypeMultiplier(move.type, target.secondaryType) / 2.0f;
		}
		type *= primaryMod * secondaryMod;
		if (type > 1.0f)
		{
			BattleTextBuffer::clearText();
			BattleTextBuffer::pushText("It was super effective!");
			PokemonBattle::awaitInput();
		}
		else if (type < 1.0f)
		{
			BattleTextBuffer::clearText();
			BattleTextBuffer::pushText("It's not very effective...");
			PokemonBattle::awaitInput();
		}
	}
	else
	{
		BattleTextBuffer::clearText();
		BattleTextBuffer::pushText(attacker.nickname + "'s " + "attack missed!");
		PokemonBattle::awaitInput();
	}

	//Status
	if (move.status == StatusCondition::None)
	{
		return;
	}

	roll = PokemonBattle::random.next();
	if (roll < move.statusAcc * 100)
	{
		BattleTextBuffer::clearText();
		BattleTextBuffer::pushText(target.nickname + " was " + std::to_string((int)move.status));
		PokemonBattle::awaitInput();
		target.condition = move.status;
		if (move.status == StatusCondition::Sleep)
		{
			target.storage = 2;
		}
	}
}

int16_t CalculateDamage(Pokemon& attacker, Pokemon& target, PokemonMove move, CurrentStages& attackerStage, CurrentStages& defenderStage)
{
	//Calc whether critical hit - 6.25% at stage one
	float critical = 1;
	float critRoll = PokemonBattle::random.next();
	if (critRoll < (625.0f*LookupStageMultiplier(attackerStage.critStage)))
	{
		critical = 2;
		BattleTextBuffer::clearText();
		BattleTextBuffer::pushText("Its a critical hit!");
		PokemonBattle::awaitInput();
	}

	//Calc random factor and clamp between 85 and 100
	float randomRoll = (85.0f + (float)PokemonBattle::random.next() * (15.0f/(float)BATTLE_PROBABILITY_MAX)) / 100.0f;

	//Calc stab
	float stab = 1.0f;
	if (attacker.primaryType == move.type || attacker.secondaryType == move.type)
	{
		stab = 1.5f;
	}

	//Calc type effectiveness
	float type = 1.0f;
	//Use primary type of attacker
	float primaryMod = (float)LookupTypeMultiplier(move.type, target.primaryType) / 2.0f;
	float secondaryMod = 1.0f;
	if (target.secondaryType != PokemonType::Null)
	{
		secondaryMod = (float)LookupTypeMultiplier(move.type, target.secondaryType) / 2.0f;
	}
	type *= primaryMod * secondaryMod;

	float level = ((2 * (float)attacker.level) / 5) + 2;
	float attackOverDefence = (LookupStageMultiplier(attackerStage.attackStage)*(float)attacker.attack) / (LookupStageMultiplier(defenderStage.defenseStage)*(float)target.defense);
	float damageOut = ((level * (float)move.damage * attackOverDefence) / 50.0f) + 2.0f;

	//Now multiply premultiplier by additional effects - easy to add
	damageOut *= critical * randomRoll * stab * type;

	//Cast to int
	return (int16_t)damageOut;
}

void ProcessEndTurnStatus(Pokemon& pokemon)
{
	uint16_t damage;
	switch (pokemon.condition)
	{
	case StatusCondition::None:
		return;
	case StatusCondition::Poison:
		//Calc damage taken
		damage = pokemon.hp / 16;
		pokemon.health -= damage;
		BattleTextBuffer::clearText();
		BattleTextBuffer::pushText(pokemon.nickname + " was hurt by poison!");
		PokemonBattle::awaitInput();
		return;
	case StatusCondition::Burn:
		//Calc damage taken
		damage = pokemon.hp / 16;
		pokemon.health -= damage;
		BattleTextBuffer::clearText();
		BattleTextBuffer::pushText(pokemon.nickname + " was hurt by burn!");
		PokemonBattle::awaitInput();
		return;
	default:
		return;
	}
}

bool ProcessStatus(Pokemon& pokemon)
{
	bool skipTurn = false;
	float roll = 0.0f;
	switch (pokemon.condition)
	{
	case StatusCondition::None:
		return false;
	case StatusCondition::Sleep:
		//Check if sleep timer (kept in storage) has hit zero - if so sleep ends
		if (pokemon.storage <= 0)
		{
			BattleTextBuffer::clearText();
			BattleTextBuffer::pushText(pokemon.nickname + " woke up!");
			PokemonBattle::awaitInput();
			pokemon.condition = StatusCondition::None;
			pokemon.storage = 0;
			return false;
		}
		//Otherwise roll for a chance to end - is 33.3% so roll 3,333/10,000
		roll = PokemonBattle::random.next();
		if (roll <= 3333.0f)
		{
			BattleTextBuffer::clearText();
			BattleTextBuffer::pushText(pokemon.nickname + " woke up!");
			PokemonBattle::awaitInput();
			pokemon.condition = StatusCondition::None;
			pokemon.storage = 0;
			return false;
		}
		BattleTextBuffer::clearText();
		BattleTextBuffer::pushText(pokemon.nickname + " is asleep...");
		PokemonBattle::awaitInput();
		pokemon.storage--;
		return true;
	case StatusCondition::Paralysis:
		//Roll for a chance to miss turn - is 25%
		roll = PokemonBattle::random.next();
		if (roll < 7500.0f)
		{
			return false;
		}
		BattleTextBuffer::clearText();
		BattleTextBuffer::pushText(pokemon.nickname+" is paralyzed! And isn't able to move!");
		PokemonBattle::awaitInput();
		return true;
	default:
		return false;
	}
}

//Move queue
void MoveQueue::queueMove(PokemonMove move, Pokemon* origin, Pokemon* target, CurrentStages* originStages, CurrentStages* targetStages)
{
	uint8_t& index = m_MoveQueue[m_QueueIndex].size;
	m_MoveQueue[m_QueueIndex].moves[index].move = move;
	m_MoveQueue[m_QueueIndex].moves[index].origin = origin;
	m_MoveQueue[m_QueueIndex].moves[index].target = target;
	m_MoveQueue[m_QueueIndex].moves[index].originStages = originStages;
	m_MoveQueue[m_QueueIndex].moves[index].targetStages = targetStages;
	index++;
}

void MoveQueue::queueMove(PokemonMove move, Pokemon* origin, Pokemon* target, CurrentStages* originStages, CurrentStages* targetStages, unsigned int turnIndex)
{
	uint8_t& index = m_MoveQueue[turnIndex].size;
	m_MoveQueue[turnIndex].moves[index].move = move;
	m_MoveQueue[turnIndex].moves[index].origin = origin;
	m_MoveQueue[turnIndex].moves[index].target = target;
	m_MoveQueue[turnIndex].moves[index].originStages = originStages;
	m_MoveQueue[turnIndex].moves[index].targetStages = targetStages;
	index++;
}

void MoveQueue::processTurn()
{
	//First sort indexes by fastest pokemon - if same speed roll to solve
	MoveTurn turn = m_MoveQueue[m_QueueIndex];

	//Sort array into speed order
	std::sort(std::begin(turn.moves), std::end(turn.moves), SortBySpeed);

	//Carry out attacks
	for (int i = 0; i < turn.size; i++)
	{
		//First process status
		if (!ProcessStatus(*turn.moves[i].origin))
		{
			ExecuteAttack(*turn.moves[i].origin, *turn.moves[i].target, turn.moves[i].move, *turn.moves[i].originStages, *turn.moves[i].targetStages);
		}
	}

	//Next turn
	m_QueueIndex++;
	if (m_QueueIndex >= MOVE_QUEUE_LENGTH)
	{
		m_QueueIndex = 0;
	}
}

//Battle text buffer
std::string BattleTextBuffer::m_Line1 = ""; std::string BattleTextBuffer::m_Line2 = "";
int BattleTextBuffer::m_CurrentLine = 1;

void BattleTextBuffer::pushText(std::string text)
{
	if (m_CurrentLine == 1)
	{
		m_Line1 = text;
		m_CurrentLine = 2;
	}
	else
	{
		m_Line2 = text;
		m_CurrentLine = 1;
	}
}

//Pokemon battle
bool* PokemonBattle::s_Progress[2] = { nullptr, nullptr };

bool PokemonBattle::checkWin()
{
	//Check if anyone is dead
	if (m_PartyA[m_ActivePkmA].health < 0)
	{
		BattleTextBuffer::clearText();
		BattleTextBuffer::pushText("Player lost!");
		PokemonBattle::awaitInput();
		return true;
	}
	else if (m_PartyB[m_ActivePkmB].health < 0)
	{
		BattleTextBuffer::clearText();
		BattleTextBuffer::pushText("Enemy lost!");
		PokemonBattle::awaitInput();
		return true;
	}
	return false;
}

void PokemonBattle::nextMove()
{
	//Execute top move
	m_MoveQueue.processTurn();
	//Check if won
	if (checkWin()) { return; }
}

void PokemonBattle::run(MoveSlot move)
{
	m_NextMoveA = move;
	if (m_NextMoveA == MoveSlot::SLOT_NULL)
	{
		return;
	}
	m_IsUpdating = true;

	//Check if won
	if (checkWin()) { m_IsUpdating = false; return; }

	//Await player input, add to queue
	m_MoveQueue.queueMove(m_PartyA[m_ActivePkmA].moves[(int)move], &m_PartyA[m_ActivePkmA], &m_PartyB[m_ActivePkmB], &m_StagesA, &m_StagesB);
	
	//Await enemy input, add to queue
	m_MoveQueue.queueMove(m_PartyB[m_ActivePkmB].moves[0], &m_PartyB[m_ActivePkmB], &m_PartyA[m_ActivePkmA], &m_StagesB, &m_StagesA);

	this->nextMove();

	//Process end turn status
	ProcessEndTurnStatus(m_PartyA[m_ActivePkmA]);
	ProcessEndTurnStatus(m_PartyB[m_ActivePkmB]);

	BattleTextBuffer::clearText();

	m_IsUpdating = false;
}

void PokemonBattle::awaitInput()
{
	while(!(*s_Progress[0] || *s_Progress[1]))
	{
		//Check every 50ms whether can continue
		//Could make efficient but it works pretty well
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	*s_Progress[0] = false; *s_Progress[1] = false; //Isn't currently thread safe
}