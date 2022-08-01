#pragma once
#ifndef BATTLE_GUI_H
#define BATTLE_GUI_H

#include <array>
#include <shared_mutex>

#include "game/gui/GUI.h"
#include "game/pokemon/Pokemon.h"
#include "game/pokemon/PokemonBattle.h"

class BattleGUI : public GameGUI::Divider
{
public:
	void setFontContainer(FontContainer* font) { m_Fonts = font; }
	void openNest();
	void closeNest();

	void setNameA(std::string name) { m_PkmNameA = name; m_PkmNameA += " "; m_PkmNameA[0] = std::toupper(m_PkmNameA[0]); }
	void setLevelA(int level) { m_PkmLvlA = "Lv " + std::to_string(level); }
	void setNameB(std::string name) { m_PkmNameB = name; m_PkmNameB += " "; m_PkmNameB[0] = std::toupper(m_PkmNameB[0]); }
	void setLevelB(int level) { m_PkmLvlB = "Lv " + std::to_string(level); }
	void setMoves(std::array<PokemonMove, 4>& moves);
	void setMoveTriggers(bool* triggers) { m_MoveTriggers = triggers; }
	void setHealthPerc(float a, float b) { std::lock_guard lock(m_AccessMutex); m_HealthPercA = a; m_HealthPercB = b; }
	void setHealthPercA(float perc) { m_HealthPercA = perc; }
	void setHealthPercB(float perc) { m_HealthPercB = perc; }

	//Link
	void linkSwitchPkm(bool* switchPkm) { m_SwitchPkm = switchPkm; }

private:
	//Style
	void guiStyle();
	void endGuiStyle();

	//Helpers
	void playerBox();
	void enemyBox();
	void textBox();

	//Text storage
	std::string m_PkmNameA = "";
	std::string m_PkmLvlA = "0";
	std::string m_PkmNameB = "";
	std::string m_PkmLvlB = "0";

	//Switch pkm
	bool* m_SwitchPkm = nullptr;

	//Health
	float m_HealthPercA; float m_HealthPercB;
	std::shared_mutex m_AccessMutex;

	//Move buttons
	bool* m_MoveTriggers = nullptr;
	int m_TriggersCount = 0;
	std::string m_MoveNames[4] = { " ", " ", " ", " " };

	//Colors
	ImVec4 getMoveColor(PokemonType type);
	const ImVec4 BUTTON_COLOR = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
	const ImVec4 WINDOW_COLOR = ImVec4(0.2f, 0.2f, 0.25f, 0.9f);
	ImVec4 m_ButtonColors[4]
	{
		BUTTON_COLOR, BUTTON_COLOR, BUTTON_COLOR, BUTTON_COLOR
	};

	FontContainer* m_Fonts = nullptr;
};

#endif