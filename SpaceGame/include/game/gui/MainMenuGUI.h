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
	void openNest();
	void closeNest();
private:
	std::function<void(bool)> m_SetActive;
	std::shared_ptr<Overworld> m_OverworldPtr;
	FontContainer* m_Fonts = nullptr;
	bool* m_ShowOptions = nullptr;
};

#endif