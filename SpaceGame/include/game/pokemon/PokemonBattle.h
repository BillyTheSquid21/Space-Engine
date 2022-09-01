#pragma once
#ifndef POKEMON_BATTLE_H
#define POKEMON_BATTLE_H

#include "utility/SGUtil.h"
#include "game/utility/GameText.h"

#include "game/pokemon/Pokemon.h"
#include "game/pokemon/StatStages.hpp"
#include "game/pokemon/Turn.hpp"
#include "game/pokemon/MoveEffects.h"
#include "utility/Random.h"
#include "game/gui/GUI.h"
#include "chrono"
#include "functional"

#define MOVE_QUEUE_LENGTH 5
#define BATTLE_PROBABILITY_MAX 10000
#define TYPE_COUNT 18

//Type effectiveness lookup for ATTACK - don't need to store defensive info
//Stored in order the types are stored above and so can be used as keys
//0 is no effect, 1 is half power, 2 is normal power, 4 is 2x power (value / 2 to get multiplier)
static const uint8_t FairyEff[TYPE_COUNT]
{
	2,1,2,2,2,2,4,1,2,2,2,2,2,2,4,4,1,2
};
static const uint8_t SteelEff[TYPE_COUNT]
{
	2,1,1,2,1,4,2,2,2,2,2,2,4,2,2,2,1,4
};
static const uint8_t DragonEff[TYPE_COUNT]
{
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,1,0
};
static const uint8_t DarkEff[TYPE_COUNT]
{
	2,2,2,2,2,2,1,2,2,2,4,2,2,4,1,2,2,1
};
static const uint8_t GhostEff[TYPE_COUNT]
{
	0,2,2,2,2,2,2,2,2,2,4,2,2,4,1,2,2,2
};
static const uint8_t RockEff[TYPE_COUNT]
{
	2,4,2,2,2,4,1,2,1,4,2,4,2,2,2,2,1,2
};
static const uint8_t BugEff[TYPE_COUNT]
{
	2,1,2,4,2,2,1,1,2,1,4,2,2,1,4,2,1,1
};
static const uint8_t PsychicEff[TYPE_COUNT]
{
	2,2,2,2,2,2,4,4,2,2,1,2,2,2,0,2,1,2
};
static const uint8_t FlyingEff[TYPE_COUNT]
{
	2,2,2,4,1,2,4,2,2,2,2,4,1,2,2,2,1,2
};
static const uint8_t GroundEff[TYPE_COUNT]
{
	2,4,2,1,4,2,2,4,2,0,2,1,4,2,2,2,4,2
};
static const uint8_t PoisonEff[TYPE_COUNT]
{
	2,2,2,4,2,2,2,1,1,2,2,2,1,1,2,2,0,4
};
static const uint8_t FightingEff[TYPE_COUNT]
{
	4,2,2,2,2,4,2,1,2,1,1,1,4,0,4,2,4,1
};
static const uint8_t NormalEff[TYPE_COUNT]
{
	2,2,2,2,2,2,2,2,2,2,2,2,1,0,2,2,1,1
};
static const uint8_t FireEff[TYPE_COUNT]
{
	2,1,1,4,2,4,2,2,2,2,2,4,1,2,2,1,4,2
};
static const uint8_t WaterEff[TYPE_COUNT]
{
	2,4,1,1,2,2,2,2,4,2,2,2,4,2,2,1,2,2
};
static const uint8_t GrassEff[TYPE_COUNT]
{
	2,1,4,1,2,2,2,1,4,1,2,1,4,2,2,1,1,2
};
static const uint8_t ElectricEff[TYPE_COUNT]
{
	2,2,4,1,1,2,2,2,0,4,2,2,2,2,2,1,2,2
};
static const uint8_t IceEff[TYPE_COUNT]
{
	2,1,1,4,2,1,2,2,4,4,2,2,2,2,2,4,1,1
};

static const uint8_t* TypeLookup[TYPE_COUNT]
{
	NormalEff, FireEff, WaterEff, GrassEff, ElectricEff, IceEff, FightingEff, PoisonEff, GroundEff, FlyingEff,
	PsychicEff, BugEff, RockEff, GhostEff, DarkEff, DragonEff, SteelEff, FairyEff
};

static uint8_t LookupTypeMultiplier(PokemonType attacking, PokemonType defending);
static float LookupStageMultiplier(Stage stage);

//Attacking
static void ExecuteAttack(TurnData& turn);

//Can use interchangably with attack and sp. attack as is now agnostic
static int16_t CalculateDamage(
	int16_t attackStat, 
	int16_t defenceStat, 
	Pokemon& attacker, 
	Pokemon& target, 
	TurnMove move, 
	Stage attackStage, 
	Stage defendStage, 
	Stage critStage
);

static bool ProcessStatus(Pokemon& pokemon);
static void ProcessEndTurnStatus(Pokemon& pokemon);

//Queue has a circular stack array of moves so a move can have an effect ahead
class MoveQueue
{
public:
	MoveQueue() { random.seed(0.0f, BATTLE_PROBABILITY_MAX); }

	void queueMove(TurnData move, int turnsAhead);
	void linkFaintCheck(std::function<void(bool)> func) { m_CheckFaint = func; }
	void processTurn();

	static const int POKEMON_COUNT = 2;
	struct Turn
	{
		TurnData moves[POKEMON_COUNT];
		uint8_t size = 0;
	};
	static SGRandom::RandomContainer random;
private:
	Turn m_MoveQueue[MOVE_QUEUE_LENGTH]; //Array of how many moves to look ahead
	unsigned int m_QueueIndex = 0; //Rolls around to make queue array circular
	std::function<void(bool)> m_CheckFaint;
};

static bool SortBySpeed(const TurnData& lhs, const TurnData& rhs);
static void QueueMove(
	MoveQueue& queue, 
	PokemonMove move,
	Pokemon* origin,
	Pokemon* target,
	CurrentStages* originStages,
	CurrentStages* targetStages,
	PokemonMoveState* originState,
	PokemonMoveState* targetState
);

class PokemonBattle
{
public:
	PokemonBattle() { random.seed(0.0f, BATTLE_PROBABILITY_MAX); m_MoveQueue.linkFaintCheck(std::bind(&PokemonBattle::checkFaint, this, std::placeholders::_1)); }
	void linkProgressButtons(bool* pressE, bool* pressX) { s_Progress[0] = pressE; s_Progress[1] = pressX; }
	void linkHealthGui(std::function<void(float, float)> func);

	void setParties(Party* playerParty, Party* enemyParty) { m_PartyA = playerParty; m_PartyB = enemyParty; m_BattleOver = false; };
	void run(MoveSlot move);
	void setActiveA(int slot) { m_ActivePkmA = slot; }
	void setActiveB(int slot) { m_ActivePkmB = slot; }

	int16_t getHealthA() const { return m_PartyA->at(m_ActivePkmA).health; }
	int16_t getHealthB() const { return m_PartyB->at(m_ActivePkmB).health; }
	int16_t getHPA() const { return m_PartyA->at(m_ActivePkmA).stats.hp; }
	int16_t getHPB() const { return m_PartyB->at(m_ActivePkmB).stats.hp; }
	float getHealthPercA() { return ((float)getHealthA()) / ((float)getHPA()); }
	float getHealthPercB() { return ((float)getHealthB()) / ((float)getHPB()); }
	int16_t getLevelA()  const { return m_PartyA->at(m_ActivePkmA).level; }
	int16_t getLevelB()  const { return m_PartyB->at(m_ActivePkmB).level; }
	uint8_t getStatusA() const { return (uint8_t)m_PartyA->at(m_ActivePkmA).condition; }
	uint8_t getStatusB() const { return (uint8_t)m_PartyB->at(m_ActivePkmB).condition; }
	std::string getNameA() const { return m_PartyA->at(m_ActivePkmA).nickname; }
	std::string getNameB() const { return m_PartyB->at(m_ActivePkmB).nickname; }
	std::array<PokemonMove, 4>& getMovesA() const { return m_PartyA->at(m_ActivePkmA).moves; }

	bool checkMoveValid(MoveSlot slot) { if (m_PartyA->at(m_ActivePkmA).moves[(int)slot].type == PokemonType::None) { return false; } return true; }
	bool isUpdating() { return m_IsUpdating; }
	bool isOver() { return m_BattleOver; }
	
	//Only to be called from battle thread - otherwise will likely lock
	static void awaitInput();
	static SGRandom::RandomContainer random;

	//Public turn helpers
	bool checkFaintA(bool textEnabled);
	bool checkFaintB(bool textEnabled);

	//Text
	static GameGUI::TextBuffer textBuffer;

private:

	//Private turn helpers
	void nextMove();
	void checkFaint(bool textEnabled); //See if any pokemon have fainted
	bool checkLossA(); //See if team A has no viable pokemon
	bool checkLossB(); //See if team B has no viable pokemon
	bool checkLoss();  //Check both
	bool replacePokemon(); //Force player or enemy to change out fainted pkm
	void nextViablePokemonB();

	static enum class Team : uint8_t
	{
		A, B
	};

	//Stack array containing next moves
	MoveQueue m_MoveQueue;

	//Bool ptr to next button
	static bool* s_Progress[2];

	//Pokemon
	Party* m_PartyA = nullptr; Party* m_PartyB = nullptr;
	unsigned int m_ActivePkmA = 0; unsigned int m_ActivePkmB = 0;
	MoveSlot m_NextMoveA = MoveSlot::SLOT_NULL;
	MoveSlot m_NextMoveB = MoveSlot::SLOT_NULL;
	CurrentStages m_StagesA; CurrentStages m_StagesB;
	PokemonMoveState m_PkmStateA = PokemonMoveState::Normal;
	PokemonMoveState m_PkmStateB = PokemonMoveState::Normal;

	//Thread
	std::atomic<bool> m_IsUpdating = false;
	std::atomic<bool> m_BattleOver = false;
	std::function<void(float,float)> m_HealthUpdate;
};

#endif
