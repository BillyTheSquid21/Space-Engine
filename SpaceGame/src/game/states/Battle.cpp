#include "game/states/Battle.h"

void Battle::init(int width, int height, FontContainer* fonts, FlagArray* flags, GameInput* input) {
    //Width and height
    m_Width = width; m_Height = height;

    //Flags
    m_Flags = flags;

    //Input
    m_Input = input;

    EngineLog("Battle scene loaded");
}

void Battle::loadRequiredData()
{
    m_PlayerParty[0].id = 0;
    m_PlayerParty[0].moves[0].type = PokemonType::Normal;
    m_PlayerParty[0].moves[0].damage = 40;
    m_EnemyParty[0].id = 0;
    m_EnemyParty[0].moves[0].type = PokemonType::Normal;
    m_EnemyParty[0].moves[0].damage = 40;
    m_Battle.setParties(m_PlayerParty, m_EnemyParty);
    m_DataLoaded = true;
}

void Battle::purgeRequiredData()
{

}

void Battle::update(double deltaTime, double time)
{
    m_Battle.run();
}

void Battle::render()
{

}

void Battle::handleInput(int key, int scancode, int action, int mods)
{
    m_Input->handleInput(key, scancode, action, mods);
}