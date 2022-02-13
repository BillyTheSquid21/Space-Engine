#include "game/gui/GUI.h"

void GameGUI::SetColors(int r, int g, int b, ImGuiCol target) {
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[target] = ImColor(r, g, b);
}

void GameGUI::SetNextWindowSize(float width, float height) {
	ImGui::SetNextWindowSize(ImVec2(width, height), 0);
}

void GameGUI::SetNextWindowPos(float x, float y) {
	ImGui::SetNextWindowPos(ImVec2(x, y), 0);
}