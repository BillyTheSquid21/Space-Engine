#pragma once
#ifndef OVERWORLD_GUI_H
#define OVERWORLD_GUI_H

#include "game/gui/GUI.h"
#include "game/data/PlayerData.hpp"
#include <cctype>

class OverworldMenu : public GameGUI::Divider
{
public:
	void setFontContainer(FontContainer* font) { m_Fonts = font; }
	void setPlayerData(PlayerData* data) { m_PlayerData = data; }
	void openNest();
	void closeNest();
private:

	//Whole menu style - call reset style at end
	void menuStyle();
	void endMenuStyle();

	//Pkm Menu
	void pkmMenu();
	void pkmDivider(int slot);
	void pkmMenuStyle();
	void endPkmMenuStyle();

	bool m_ShowPkmMenu = false;
	FontContainer* m_Fonts = nullptr;
	PlayerData* m_PlayerData = nullptr;
};

#endif