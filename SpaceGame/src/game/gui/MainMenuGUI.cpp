#include "game/gui/MainMenuGUI.h"

void MainMenuGUI::openNest()
{
	GameGUI::Divider::openNest();
    ImGui::PushFont(m_Fonts->getFont("default", 50));
    ImGui::SetCursorPosX(18.0f);
    ImGui::Text("Demo Game");
    ImGui::PopFont();


    //Buttons
    ImGui::PushFont(m_Fonts->getFont("default", 25));
    if (ImGui::Button("Game", ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 50))) {
        clickSound();
        m_OverworldPtr->setActive(true);
        m_OverworldPtr->loadRequiredData();
        m_SetActive(false);
    }
    if (ImGui::Button("Options", ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 50))) {
        clickSound();
        *m_ShowOptions = !(*m_ShowOptions);
    }
    if (ImGui::Button("Exit", ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 50))) {
        clickSound();
        Game::s_Close = true;
    }
    ImGui::PopFont();
}

void MainMenuGUI::closeNest()
{
	GameGUI::Divider::closeNest();
}

void MainMenuGUI::clickSound()
{
    m_System->playSound(m_Sound, &m_ClickChannel[m_CurrentChannel], SGSound::ChannelGroup::EFFECTS);
    m_CurrentChannel = !m_CurrentChannel;
}