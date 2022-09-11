#include "game/gui/OptionsGUI.h"

void OptionsMenu::openNest()
{
	if (!m_Show)
	{
		return;
	}
	Divider::openNest();

	ImGui::Text("Audio Settings");

	//Effect volume slider
	Options::effects->getVolume(&m_EffectVolume);
	ImGui::SliderFloat("Effects Volume", &m_EffectVolume, 0.0f, 1.0f);
	Options::effects->setVolume(m_EffectVolume);

	//Music volume
	Options::music->getVolume(&m_MusicVolume);
	ImGui::SliderFloat("Music Volume", &m_MusicVolume, 0.0f, 1.0f);
	Options::music->setVolume(m_MusicVolume);

	ImGui::Text("Graphics Settings");

	//Grass Density
	ImGui::SliderInt("Grass Density", &Options::grassDensity, 10, 40);
}

void OptionsMenu::closeNest()
{
	if (!m_Show)
	{
		return;
	}
	Divider::closeNest();
}
