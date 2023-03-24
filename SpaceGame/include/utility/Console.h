#pragma once
#ifndef CONSOLE_H
#define CONSOLE_H

#include "core/GUI.h"
#include "core/Root.h"
#include "GLFW/glfw3.h"
#include "textflow/TextFlow.hpp"
#include "string"
#include "utility/NPlot.hpp"
#include "imgui/implot.h"

#define CONSOLE_ENABLE 1

namespace SGRoot
{
	//ID for which plot to add to - plots are static and always present
	enum class PlotID
	{
		LightTime
	};

	class ConsoleWindow : public SGGUI::GUIBase
	{
	public:
		void start(float xOff, float yOff, float screenW, float screenH);
		void end();
		void handleInput(int key, int scancode, int action, int mods);
	private:
		std::string m_ConsoleBuffer = ">console Begin";
		std::string m_ConsoleBufferWrapped = ">console begin";
		std::string m_CurrentLineBuffer = "";
		std::string m_LineBufferWrapped = "";
		bool m_Typing = false;
		bool m_ShowConsole = false;

		NPlot<double, 2> m_FPSPlot = NPlot<double, 2>::NPlot(600); //Records 600 back
		size_t m_FrameCount = 0; //X Axis of fps plot
	};
}

#endif