#pragma once
#ifndef OVERWORLD_GUI_H
#define OVERWORLD_GUI_H

#include "game/gui/GUI.h"
#include "game/data/PlayerData.hpp"
#include "renderer/Texture.h"
#include <cctype>

class OverworldMenu : public GameGUI::Divider
{
public:
	void setFontContainer(FontContainer* font) { m_Fonts = font; }
	void setPlayerData(PlayerData* data) { m_PlayerData = data; }
	void openNest();
	void closeNest();
private:
	//Helpers
	void unloadAllIcons();

	//Whole menu style - call reset style at end
	void menuStyle();
	void endMenuStyle();

	//Pkm Menu
	void pkmMenu();
	void pkmDivider(int slot);
	void pkmMenuStyle();
	void endPkmMenuStyle();

	//Bag
	enum class BagPocket
	{
		Items = 1, Medicine = 2, Pokeballs = 3, TMs = 4, Berries = 5, KeyItems = 8
	};

	void bag();
	void bagButtons();
	void bagColor();
	void endBagColor();
	void bagItems();
	void bagInnerStyle();
	void endBagInnerStyle();
	void loadBagIcon(Texture& texture, uint16_t id);
	void getBagIconPath(std::string& path, uint16_t id);
	BagPocket m_Pocket = BagPocket::Items;
	bool m_JustClicked = false;
	
	Texture m_Icons[12];
	float m_LastScroll = -1.0f;
	BagPocket m_LastPocket = BagPocket::Items;
	bool m_AwaitingIconLoad = false;
	bool m_IconsLoaded = false;

	bool m_ShowPkmMenu = false;
	bool m_ShowBag = false;
	FontContainer* m_Fonts = nullptr;
	PlayerData* m_PlayerData = nullptr;
};

#endif