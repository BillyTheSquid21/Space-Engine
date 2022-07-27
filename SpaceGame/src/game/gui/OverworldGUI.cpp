#include "game/gui/OverworldGUI.h"

//Overworld menu
void OverworldMenu::openNest()
{
	if (!m_Show)
	{
		return;
	}
	menuStyle();
	if (m_ShowPkmMenu)
	{
		pkmMenu();
	}
	Divider::openNest();
	ImGui::PushFont(m_Fonts->getFont("boxfont", 45));
	if (ImGui::Button("Pokemon", ImVec2(ImGui::GetContentRegionAvail().x, 75.0f)))
	{
		m_ShowPkmMenu = !m_ShowPkmMenu;
	}
	endMenuStyle();
}

void OverworldMenu::menuStyle()
{
	//Style
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 118.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 40.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 4.0f);

	//Color
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.25f, 0.9f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.35f, 1.0f));
}

void OverworldMenu::endMenuStyle()
{
	//Style
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	//Color
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void OverworldMenu::pkmMenu()
{
	pkmMenuStyle();
	ImGui::BeginChild("PkmMenu", ImVec2(m_Width * 5.2f, ImGui::GetContentRegionAvail().y), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	
	//Top divider
	ImGui::PushFont(m_Fonts->getFont("boxfont", 45));
	for (int i = 0; i < 6; i++)
	{
		pkmDivider(i);
	}
	ImGui::PopFont();
	ImGui::EndChild();
	endPkmMenuStyle();
}

void OverworldMenu::pkmDivider(int slot)
{
	//Check pokemon in slot
	if (m_PlayerData->playerParty[slot].id != -1)
	{
		//Create text - will probably make more efficient if needed
		std::string text = "";
		text += m_PlayerData->playerParty[slot].nickname;
		text += " Lv: " + std::to_string(m_PlayerData->playerParty[slot].level) + "\n";
		text[0] = std::toupper(text[0]);
		text += "Health: " + std::to_string(m_PlayerData->playerParty[slot].health) + "\n";
		text += "Status: " + std::to_string((int)m_PlayerData->playerParty[slot].condition);

		ImGui::Button(text.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, m_Height / 6.2f));
	}
}

void OverworldMenu::pkmMenuStyle()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 36.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.01f, 0.25f));
}

void OverworldMenu::endPkmMenuStyle()
{
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void OverworldMenu::closeNest()
{
	if (!m_Show)
	{
		return;
	}
	ImGui::PopFont();
	Divider::closeNest();
}