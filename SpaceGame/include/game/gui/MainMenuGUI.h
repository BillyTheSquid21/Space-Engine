#pragma once
#ifndef MAIN_MENU_GUI_H
#define MAIN_MENU_GUI_H

#include "core/Sound.h"
#include "game/gui/GUI.h"
#include "game/states/Overworld.h"
#include "functional"
#include "core/Game.h"

class MainMenuGUI : public GameGUI::Divider
{
public:
	void setFontContainer(FontContainer* font) { m_Fonts = font; }
	void setOverworldPtr(std::shared_ptr<Overworld> ptr) { m_OverworldPtr = ptr; }
	void linkActiveFunc(std::function<void(bool)> func) { m_SetActive = func; }
	void linkShowOptions(bool* opt) { m_ShowOptions = opt; }
	void linkSoundSystem(SGSound::System* system) { m_System = system; m_Sound = m_System->loadSound("res/sound/ui/select.wav"); }
	void openNest();
	void closeNest();
private:
	std::function<void(bool)> m_SetActive;
	std::shared_ptr<Overworld> m_OverworldPtr;
	FontContainer* m_Fonts = nullptr;
	bool* m_ShowOptions = nullptr;

	//Sound effects
	SGSound::System* m_System = nullptr;

	//Click
	SGSound::sound_id m_Sound;
	FMOD::Channel* m_ClickChannel[2] = { nullptr, nullptr };
	char m_CurrentChannel = 0;

	void clickSound();
};

#endif