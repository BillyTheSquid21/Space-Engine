#pragma once
#ifndef OPTIONS_GUI_H
#define OPTIONS_GUI_H

#include "core/Sound.h"
#include "game/data/Options.h"
#include "game/gui/GUI.h"

class OptionsMenu : public GameGUI::Divider
{
public:
	void setFontContainer(FontContainer* font) { m_Fonts = font; }
	void openNest();
	void closeNest();
private:
	void menuStyle();
	void endMenuStyle();
	float m_EffectVolume = 1.0f;
	float m_MusicVolume = 1.0f;
	float m_GrassDensity = 40.0f;
	FontContainer* m_Fonts = nullptr;
};

#endif