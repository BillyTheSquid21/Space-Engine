#include "utility/Console.h"

#define FONT_SIZE 13
#define WINDOW_WIDTH 400
#define INFO_WINDOW_WIDTH 375

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

	//Plot fps
	const double fps_60 = 0.01666;
	double fps_max_recorded = m_FPSPlot.MaxN(1);
	double fps_min_recorded = m_FPSPlot.MinN(1);
	double frame_max_recorded = m_FPSPlot.MaxN(0);
	double frame_min_recorded = m_FPSPlot.MinN(0);
	double fps_max = ((fps_60 + 0.01) * (fps_max_recorded < fps_60)) + (fps_max_recorded * (fps_max_recorded >= fps_60));

	ImPlot::SetNextAxesLimits(frame_min_recorded, frame_max_recorded, 0, fps_max, ImPlotCond_Always);
	if (m_FrameCount > 0 && ImPlot::BeginPlot("Frametime"))
	{
		ImPlot::PlotLine("Frametime", m_FPSPlot.NData(0), m_FPSPlot.NData(1), m_FPSPlot.size());
		
		//Plot target fps of 60, 30, 15
		double xLine[2] = { frame_min_recorded, frame_max_recorded };
		double yLine60[2] = { fps_60, fps_60 };
		double yLine30[2] = { fps_60*2.0, fps_60*2.0 };
		double yLine15[2] = { fps_60*4.0, fps_60*4.0 };
		ImPlot::PlotLine("60 FPS", xLine, yLine60, 2);
		ImPlot::PlotLine("30 FPS", xLine, yLine30, 2);
		ImPlot::PlotLine("15 FPS", xLine, yLine15, 2);
		
		ImPlot::EndPlot();
	}

	if (ImGui::Button("Show Console", ImVec2(150, 40)))
	{
		m_ShowConsole = !m_ShowConsole;
	}

	ImGui::EndChild();

	m_FPSPlot.addPoint(m_FrameCount, SGRoot::FRAME_TIME);

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

	//Track FPS
	m_FrameCount++;
}