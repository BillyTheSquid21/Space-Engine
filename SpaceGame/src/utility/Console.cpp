#include "utility/Console.h"

void SGRoot::ConsoleWindow::start(float xOff, float yOff, float screenW, float screenH)
{
	ImGui::SetCursorPos(ImVec2(xOff, yOff));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(PALETTE2_BLACK_CORAL, 0.45f));
	if (m_ShowConsole)
	{
		ImGui::BeginChild("root_window", ImVec2(400, MIN_HEIGHT), false);
	}
	else
	{
		ImGui::BeginChild("root_window", ImVec2(400, 128), false);
	}

	//Font
	auto font = SGGUI::FontStorage::getFont("menu", 24);
	ImGui::PushFont(font);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(PALETTE2_GHOST_WHITE, 0.95f));

	//Info window
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(PALETTE2_ORANGE_WEB, 0.75f));
	ImGui::BeginChild("info_window", ImVec2(ImGui::GetContentRegionAvail().x, 128));
	ImGui::Text(("FPS: " + std::to_string((int)SGRoot::FRAMERATE)).c_str());
	if (ImGui::Button("Show Console", ImVec2(150, 40)))
	{
		m_ShowConsole = !m_ShowConsole;
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();

	if (m_ShowConsole)
	{
		//Console window
		const float textEntryHeight = 60.0f;
		//Invisible child to hold text
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::BeginChild("console_output", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - textEntryHeight));
		ImGui::Text(m_ConsoleBufferWrapped.c_str());
		ImGui::EndChild();
		ImGui::PopStyleColor();

		//Button to act as pressable text entry
		if (ImGui::Button((">" + m_LineBufferWrapped).c_str(), ImGui::GetContentRegionAvail()))
		{
			m_Typing = !m_Typing;
		}
	}
	
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopFont();
}

void SGRoot::ConsoleWindow::handleInput(int key, int scancode, int action, int mods)
{
	if (!m_Typing)
	{
		return;
	}

	char character = (char)key;
	if (character >= 32 && character < 65)
	{
		if (action == GLFW_PRESS)
		{
			m_CurrentLineBuffer += character;
			m_LineBufferWrapped = WrapTextbox(m_CurrentLineBuffer, 400, 24);
		}
	}
	else if (character >= 65 && character < 90)
	{
		if (action == GLFW_PRESS)
		{
			m_CurrentLineBuffer += (character + 32);
			m_LineBufferWrapped = WrapTextbox(m_CurrentLineBuffer, 400, 24);
		}
	}
	else if (key == GLFW_KEY_BACKSPACE)
	{
		if (action == GLFW_PRESS && m_CurrentLineBuffer.length() > 0)
		{
			m_CurrentLineBuffer = m_CurrentLineBuffer.substr(0, m_CurrentLineBuffer.size() -1);
			m_LineBufferWrapped = WrapTextbox(m_CurrentLineBuffer, 400, 24);
		}
	}
	else if (key == GLFW_KEY_ENTER)
	{
		if (action == GLFW_PRESS)
		{
			//Check if just want to clear
			if (m_CurrentLineBuffer == "clear")
			{
				m_ConsoleBuffer = ">console begin\n";
				m_CurrentLineBuffer = "";
				m_ConsoleBufferWrapped = m_ConsoleBuffer;
				m_LineBufferWrapped = "";
				m_Typing = false;
				return;
			}

			//Split into arguments as well as possible, then execute
			std::vector<std::string> args;
			std::string output;
			if (SplitStringToWords(m_CurrentLineBuffer, args))
			{
				SGRoot::ExecuteCommand(args, output);
			}

			m_ConsoleBuffer += "\n>" + m_CurrentLineBuffer;
			m_ConsoleBuffer += "\n>" + output;
			m_ConsoleBufferWrapped = WrapTextbox(m_ConsoleBuffer, 400, 24);
			m_CurrentLineBuffer = "";
			m_Typing = false;
		}
	}
}

void SGRoot::ConsoleWindow::end()
{
	ImGui::EndChild();
}