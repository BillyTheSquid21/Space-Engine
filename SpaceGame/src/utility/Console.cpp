#include "utility/Console.h"

void SGRoot::ConsoleWindow::start(float xOff, float yOff, float screenW, float screenH)
{
	ImGui::SetCursorPos(ImVec2(xOff, yOff));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(PALETTE2_BLACK_CORAL, 0.45f));
	ImGui::BeginChild("root_window", ImVec2(400, 700), false);

	//Font
	auto font = SGGUI::FontStorage::getFont("menu", 24);
	ImGui::PushFont(font);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(PALETTE2_GHOST_WHITE, 0.95f));

	//Info window
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(PALETTE2_ORANGE_WEB, 0.75f));

	ImGui::BeginChild("info_window", ImVec2(ImGui::GetContentRegionAvail().x, 128));
	
	ImGui::Text(("FPS: " + std::to_string((int)SGRoot::FRAMERATE)).c_str());

	ImGui::EndChild();

	ImGui::PopStyleColor();
	
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopFont();
}

void SGRoot::ConsoleWindow::end()
{
	ImGui::EndChild();
}