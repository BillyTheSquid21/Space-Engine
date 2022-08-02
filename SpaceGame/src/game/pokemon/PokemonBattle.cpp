#include "game/pokemon/PokemonBattle.h"

RandomContainer PokemonBattle::random;
RandomContainer MoveQueue::random;

bool SortBySpeed(const TurnData& lhs, const TurnData& rhs)
{
	if (lhs.move.id == 0)
	{
		return 0;
	}
	return lhs.origin->stats.speed > rhs.origin->stats.speed;
}

uint8_t LookupTypeMultiplier(PokemonType attacking, PokemonType defending)
{
	return TypeLookup[(int)attacking][(int)defending];
}

float LookupStageMultiplier(Stage stage)
{
	return StatStageLookup[(int)stage];
}

static void CheckSuperEffective(TurnData move)
{
	//Check if was SE
		//Calc type effectiveness
	float type = 1.0f;
	//Use primary type of attacker
	float primaryMod = (float)LookupTypeMultiplier(move.move.type, move.target->primaryType) / 2.0f;
	float secondaryMod = 1.0f;
	if (move.target->secondaryType != PokemonType::None)
	{
		secondaryMod = (float)LookupTypeMultiplier(move.move.type, move.target->secondaryType) / 2.0f;
	}
	type *= primaryMod * secondaryMod;
	if (type > 1.0f)
	{
		PokemonBattle::textBuffer.pushBuffer("It was super effective!\n");
		PokemonBattle::awaitInput();
	}
	else if (type < 1.0f && type > 0.0f)
	{
		PokemonBattle::textBuffer.pushBuffer("It's not very effective...\n");
		PokemonBattle::awaitInput();
	}
	else if (type == 0.0f)
	{
		PokemonBattle::textBuffer.pushBuffer("The enemy was immune!\n");
		PokemonBattle::awaitInput();
	}
}

static int16_t ExecuteDamage(TurnData turn)
{
	if (turn.move.damage <= 0)
	{
		return 0;
	}

	int16_t damage = 0;
	switch (turn.move.damageType)
	{
	case DamageType::Physical:
		damage = CalculateDamage(
			turn.origin->stats.attack,
			turn.target->stats.defense,
			*turn.origin,
			*turn.target,
			turn.move,
			turn.originStages->attackStage,
			turn.targetStages->defenseStage,
			turn.originStages->critStage
		);
		break;
	case DamageType::Special:
		damage = CalculateDamage(
			turn.origin->stats.spAttack,
			turn.target->stats.spDefense,
			*turn.origin,
			*turn.target,
			turn.move,
			turn.originStages->spAttackStage,
			turn.targetStages->spDefenseStage,
			turn.originStages->critStage
		);
		break;
	}

	turn.target->health -= damage;
	return damage;
}

void ExecuteAttack(TurnData& turn)
{
	PokemonBattle::textBuffer.pushBuffer(turn.message);
	PokemonBattle::awaitInput();

	//Check for pre effect and handle any text
	PreMoveEffect(turn, PokemonBattle::random, PokemonBattle::textBuffer);
	if (!PokemonBattle::textBuffer.isReady())
	{
		PokemonBattle::awaitInput();
	}
	if (turn.move.skipAccuracy)
	{
		if (ExecuteDamage(turn) > 0)
		{
			CheckSuperEffective(turn);
		}
		return;
	}

	//Damage
	//Roll if misses
	float roll = PokemonBattle::random.next();
	if (roll < turn.move.accuracy * 100 && !CheckPkmStateMiss(*turn.targetState))
	{
		if (ExecuteDamage(turn) > 0)
		{
			CheckSuperEffective(turn);
		}

		//Check for post effect and handle any text
		PostMoveEffect(turn, PokemonBattle::random, PokemonBattle::textBuffer);
		if (!PokemonBattle::textBuffer.isReady())
		{
			PokemonBattle::awaitInput();
		}
	}
	else
	{
		PokemonBattle::textBuffer.pushBuffer(turn.origin->nickname + "'s " + "attack missed!\n");
		PokemonBattle::awaitInput();
	}
}

int16_t CalculateDamage(int16_t attackStat, int16_t defenceStat, Pokemon& attacker, Pokemon& target, TurnMove move, Stage attackStage, Stage defendStage, Stage critStage)
{
	//Calc whether critical hit - 6.25% at stage one
	float critical = 1;
	float critRoll = PokemonBattle::random.next();
	if (critRoll < (625.0f*LookupStageMultiplier(critStage)))
	{
		critical = 2;
		PokemonBattle::textBuffer.pushBuffer("Its a critical hit!\n");
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
	if (target.secondaryType != PokemonType::None)
	{
		secondaryMod = (float)LookupTypeMultiplier(move.type, target.secondaryType) / 2.0f;
	}
	type *= primaryMod * secondaryMod;

	float level = ((2 * (float)attacker.level) / 5) + 2;
	float attackOverDefence = (LookupStageMultiplier(attackStage)*(float)attackStat) / (LookupStageMultiplier(defendStage)*(float)defenceStat);
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
		damage = pokemon.stats.hp / 16;
		pokemon.health -= damage;
		PokemonBattle::textBuffer.clear();
		PokemonBattle::textBuffer.pushBuffer(pokemon.nickname + " was hurt by poison!\n");
		PokemonBattle::awaitInput();
		return;
	case StatusCondition::Burn:
		//Calc damage taken
		damage = pokemon.stats.hp / 16;
		pokemon.health -= damage;
		PokemonBattle::textBuffer.clear();
		PokemonBattle::textBuffer.pushBuffer(pokemon.nickname + " was hurt by burn!\n");
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
			PokemonBattle::textBuffer.clear();
			PokemonBattle::textBuffer.pushBuffer(pokemon.nickname + " woke up!\n");
			PokemonBattle::awaitInput();
			pokemon.condition = StatusCondition::None;
			pokemon.storage = 0;
			return false;
		}
		//Otherwise roll for a chance to end - is 33.3% so roll 3,333/10,000
		roll = PokemonBattle::random.next();
		if (roll <= 3333.0f)
		{
			PokemonBattle::textBuffer.pushBuffer(pokemon.nickname + " woke up!\n");
			PokemonBattle::awaitInput();
			pokemon.condition = StatusCondition::None;
			pokemon.storage = 0;
			return false;
		}
		PokemonBattle::textBuffer.pushBuffer(pokemon.nickname + " is asleep...\n");
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
		PokemonBattle::textBuffer.pushBuffer(pokemon.nickname+" is paralyzed! And isn't able to move!\n");
		PokemonBattle::awaitInput();
		return true;
	default:
		return false;
	}
}

//Move queue
void MoveQueue::queueMove(TurnData move, int turnsAhead)
{
	int queueIndex = m_QueueIndex;
	//Move ahead to move index
	for (int i = 0; i < turnsAhead; i++)
	{
		queueIndex++;
		if (queueIndex >= MOVE_QUEUE_LENGTH)
		{
			queueIndex = 0;
		}
	}

	uint8_t& index = m_MoveQueue[queueIndex].size;

	m_MoveQueue[queueIndex].moves[index] = move;
	index++;
}

void MoveQueue::processTurn()
{
	//First sort indexes by fastest pokemon - if same speed roll to solve
	Turn turn = m_MoveQueue[m_QueueIndex];

	//Sort array into speed order
	std::sort(std::begin(turn.moves), std::end(turn.moves), SortBySpeed);

	//Carry out attacks
	for (int i = 0; i < turn.size; i++)
	{
		//First process status
		if (turn.moves[i].move.id == 0)
		{
			continue;
		}
		if (!ProcessStatus(*turn.moves[i].origin))
		{
			ExecuteAttack(turn.moves[i]);
			m_CheckFaint(true);
			if (turn.moves[i].target->health <= 0)
			{
				break;
			}
		}
	}

	//Clear current turn
	Turn blankTurn;
	m_MoveQueue[m_QueueIndex] = blankTurn;

	//Next turn
	m_QueueIndex++;
	if (m_QueueIndex >= MOVE_QUEUE_LENGTH)
	{
		m_QueueIndex = 0;
	}
}

//Queue a move
void QueueMove(MoveQueue& queue, PokemonMove move, Pokemon* origin, Pokemon* target, CurrentStages* originStages, CurrentStages* targetStages, 
	PokemonMoveState* originState, PokemonMoveState* targetState)
{
	//Otherwise pass through additional processing
	std::function<void(TurnData, int)> queueFunc = std::bind(&MoveQueue::queueMove, &queue, std::placeholders::_1, std::placeholders::_2);
	QueueMovesWithEffects(queueFunc, move, origin, target, originStages, targetStages, originState, targetState);
}

//Battle text buffer
GameGUI::TextBuffer PokemonBattle::textBuffer;

//Pokemon battle
bool* PokemonBattle::s_Progress[2] = { nullptr, nullptr };

bool PokemonBattle::checkLossA()
{
	//Check if everyone is dead
	for (int i = 0; i < m_PartyA->size(); i++)
	{
		if (m_PartyA->at(i).id == -1)
		{
			continue;
		}
		else if (m_PartyA->at(i).health > 0)
		{
			return false;
		}
	}
	PokemonBattle::textBuffer.pushBuffer("Party A has lost!\n");
	PokemonBattle::awaitInput();
	return true;
}

bool PokemonBattle::checkLossB()
{
	//Check if everyone is dead
	for (int i = 0; i < m_PartyB->size(); i++)
	{
		if (m_PartyB->at(i).id == -1)
		{
			continue;
		}
		else if (m_PartyB->at(i).health > 0)
		{
			return false;
		}
	}
	PokemonBattle::textBuffer.pushBuffer("Party B has lost!\n");
	PokemonBattle::awaitInput();
	return true;
}

bool PokemonBattle::checkLoss()
{
	if (checkLossA())
	{
		return true;
	}
	else if (checkLossB())
	{
		return true;
	}
	return false;
}

bool PokemonBattle::checkFaintA(bool textEnabled)
{
	if (m_PartyA->at(m_ActivePkmA).health <= 0)
	{
		if (!textEnabled)
		{
			return true;
		}
		PokemonBattle::textBuffer.pushBuffer(m_PartyA->at(m_ActivePkmA).nickname + " fainted!\n");
		PokemonBattle::awaitInput();
		return true;
	}
	return false;
}

bool PokemonBattle::checkFaintB(bool textEnabled)
{
	if (m_PartyB->at(m_ActivePkmB).health <= 0)
	{
		if (!textEnabled)
		{
			return true;
		}
		PokemonBattle::textBuffer.pushBuffer(m_PartyB->at(m_ActivePkmB).nickname + " fainted!\n");
		PokemonBattle::awaitInput();
		return true;
	}
	return false;
}

void PokemonBattle::checkFaint(bool textEnabled)
{
	//Check if anyone is dead and update gui
	m_HealthUpdate(getHealthPercA(), getHealthPercB());
	checkFaintA(textEnabled); checkFaintB(textEnabled);
}

bool PokemonBattle::replacePokemon()
{
	if (checkFaintA(false))
	{
		//Force player to switch
		PokemonBattle::textBuffer.pushBuffer("Fainted pokemon must be switched!\n");
		PokemonBattle::awaitInput();
		PokemonBattle::textBuffer.clear();
		return true;
	}
	if (checkFaintB(false))
	{
		//Pick next viable enemy pkm
		nextViablePokemonB();
	}
	return false;
}

void PokemonBattle::nextViablePokemonB()
{
	for (int i = 0; i < m_PartyB->size(); i++)
	{
		if (m_PartyB->at(i).id != -1 && m_PartyB->at(i).health > 0)
		{
			m_ActivePkmB = i;
			return;
		}
	}
}

void PokemonBattle::nextMove()
{
	//Execute top move
	m_MoveQueue.processTurn();
}

void PokemonBattle::run(MoveSlot move)
{
	m_NextMoveA = move;
	if (m_NextMoveA == MoveSlot::SLOT_NULL)
	{
		return;
	}
	m_IsUpdating = true;

	//Update Gui
	m_HealthUpdate(getHealthPercA(),getHealthPercB());

	//Check if won
	if (checkLossA() || checkLossB()) { m_BattleOver = true; m_IsUpdating = false; return; }

	//Check if need to switch out pkm
	if (replacePokemon()) { m_IsUpdating = false; return; }

	//Await player input, add to queue
	if (move != MoveSlot::SLOT_CHANGE && move != MoveSlot::SLOT_ITEM && !CheckPkmStateMiss(m_PkmStateA))
	{
		QueueMove(m_MoveQueue, m_PartyA->at(m_ActivePkmA).moves[(int)move], &m_PartyA->at(m_ActivePkmA), &m_PartyB->at(m_ActivePkmB), &m_StagesA, &m_StagesB,
		&m_PkmStateA, &m_PkmStateB);
	}
	
	//Await enemy input, add to queue
	if (!CheckPkmStateMiss(m_PkmStateB))
	{
		QueueMove(m_MoveQueue, m_PartyB->at(m_ActivePkmB).moves[0], &m_PartyB->at(m_ActivePkmB), &m_PartyA->at(m_ActivePkmA), &m_StagesB, &m_StagesA,
			&m_PkmStateB, &m_PkmStateA);
	}
	
	this->nextMove();

	//Process end turn status
	ProcessEndTurnStatus(m_PartyA->at(m_ActivePkmA));
	ProcessEndTurnStatus(m_PartyB->at(m_ActivePkmB));

	PokemonBattle::textBuffer.clear();

	//Check if won
	if (checkLossA() || checkLossB()) { m_BattleOver = true; m_IsUpdating = false; return; }

	m_IsUpdating = false;
}

void PokemonBattle::linkHealthGui(std::function<void(float,float)> func)
{
	m_HealthUpdate = func;
}

void PokemonBattle::awaitInput()
{
	while (!textBuffer.isReady())
	{
		if ((*s_Progress[0] || *s_Progress[1]))
		{
			if (textBuffer.isNextPageReady())
			{
				textBuffer.nextPage();
			}
			*s_Progress[0] = false; *s_Progress[1] = false;
		}
		//Check every 50ms whether can continue
		//Could make efficient but it works pretty well
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	while (!(*s_Progress[0] || *s_Progress[1]))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	*s_Progress[0] = false; *s_Progress[1] = false; //Isn't currently thread safe
}