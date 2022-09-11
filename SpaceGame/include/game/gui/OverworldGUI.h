#pragma once
#ifndef OVERWORLD_GUI_H
#define OVERWORLD_GUI_H

#include "core/Sound.h"
#include "game/gui/GUI.h"
#include "game/data/PlayerData.hpp"
#include "renderer/Texture.h"
#include "cctype"

class OverworldMenu : public GameGUI::Divider
{
public:
	~OverworldMenu() { m_System->releaseSound(m_ClickEffect); }
	void setFontContainer(FontContainer* font) { m_Fonts = font; }
	void setPlayerData(PlayerData* data) { m_PlayerData = data; }
	void linkSoundSystem(SGSound::System* system) { m_System = system; m_System->loadSound(m_ClickPath.c_str(), m_ClickEffect); }
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
	int m_LastSelectedItem = -1;
	bool m_AwaitingIconLoad = false;
	bool m_IconsLoaded = false;

	bool m_ShowPkmMenu = false;
	bool m_ShowBag = false;
	FontContainer* m_Fonts = nullptr;
	PlayerData* m_PlayerData = nullptr;

	//Sound effects
	SGSound::System* m_System = nullptr;

	//Click
	std::string m_ClickPath = "res/sound/ui/select.wav";
	FMOD::Sound* m_ClickEffect = NULL;
	FMOD::Channel* m_ClickChannel = NULL;

	void clickSound() { m_System->playSound(m_ClickEffect, m_ClickChannel, SGSound::ChannelGroup::EFFECTS); };
};

#endif