#include "utility/Console.h"

#define FONT_SIZE 13
#define WINDOW_WIDTH 400
#define INFO_WINDOW_WIDTH 128

void SGRoot::ConsoleWindow::start(float xOff, float yOff, float screenW, float screenH)
{
	ImGui::SetCursorPos(ImVec2(xOff, yOff));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(PALETTE2_BLACK_CORAL, 0.45f));
	if (m_ShowConsole)
	{
		ImGui::BeginChild("root_window", ImVec2(WINDOW_WIDTH, MIN_HEIGHT), false);
	}
	else
	{
		ImGui::BeginChild("root_window", ImVec2(WINDOW_WIDTH, INFO_WINDOW_WIDTH), false);
	}

	//Font
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(PALETTE2_GHOST_WHITE, 0.95f));

	//Info window
	ImGui::BeginChild("info_window", ImVec2(ImGui::GetContentRegionAvail().x, INFO_WINDOW_WIDTH));
	ImGui::Text(("FPS: " + std::to_string((int)SGRoot::FRAMERATE)).c_str());
	if (ImGui::Button("Show Console", ImVec2(150, 40)))
	{
		m_ShowConsole = !m_ShowConsole;
	}

	//Plot fps
	if (m_FrameCount > 0 && ImPlot::BeginPlot("FPSPlot"))
	{
		//TODO - figure out this shitty plotting system
		ImPlot::EndPlot();
	}

	ImGui::EndChild();

	if (m_FrameCount % 60 == 0)
	{
		m_FPSPlot.addPoint(m_FrameCount/60, SGRoot::FRAMERATE);
	}

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
			m_LineBufferWrapped = WrapTextbox(m_CurrentLineBuffer, WINDOW_WIDTH, FONT_SIZE);
		}
	}
	else if (character >= 65 && character < 90)
	{
		if (action == GLFW_PRESS)
		{
			m_CurrentLineBuffer += (character + 32);
			m_LineBufferWrapped = WrapTextbox(m_CurrentLineBuffer, WINDOW_WIDTH, FONT_SIZE);
		}
	}
	else if (key == GLFW_KEY_BACKSPACE)
	{
		if (action == GLFW_PRESS && m_CurrentLineBuffer.length() > 0)
		{
			m_CurrentLineBuffer = m_CurrentLineBuffer.substr(0, m_CurrentLineBuffer.size() -1);
			m_LineBufferWrapped = WrapTextbox(m_CurrentLineBuffer, WINDOW_WIDTH, FONT_SIZE);
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
			m_ConsoleBufferWrapped = WrapTextbox(m_ConsoleBuffer, WINDOW_WIDTH, FONT_SIZE);
			m_CurrentLineBuffer = "";
			m_Typing = false;
		}
	}
}

void SGRoot::ConsoleWindow::end()
{
	ImGui::EndChild();
	m_FrameCount++;
}