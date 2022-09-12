#include "game/gui/OptionsGUI.h"

void OptionsMenu::openNest()
{
	if (!m_Show)
	{
		return;
	}
	menuStyle();
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
	endMenuStyle();
}

void OptionsMenu::closeNest()
{
	if (!m_Show)
	{
		return;
	}
	Divider::closeNest();
}

void OptionsMenu::menuStyle()
{
	//Style
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 118.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 40.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 4.0f);

	//Color
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.25f, 0.9f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.36f, 0.4f, 0.45f, 0.85f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.996f, 0.694f, 0.224f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.996f, 0.794f, 0.324f, 1.0f));
}

void OptionsMenu::endMenuStyle()
{
	//Style
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	//Color
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}