#include "game/objects/OverworldScript.h"

GameInput* OverworldScript::m_Input = nullptr;
PlayerData* OverworldScript::m_Data = nullptr;
std::function<void(World::LevelID)> OverworldScript::m_LoadLv;
std::function<void(World::LevelID)> OverworldScript::m_UnloadLv;