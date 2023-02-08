#pragma once
#ifndef CONSOLE_H
#define CONSOLE_H

#include "core/GUI.h"
#include "core/Root.h"
#include "GLFW/glfw3.h"
#include "string"

namespace SGRoot
{
	class ConsoleWindow : public SGGUI::GUIBase
	{
	public:
		void start(float xOff, float yOff, float screenW, float screenH);
		void end();
		void handleInput(int key, int scancode, int action, int mods);
	private:
		std::string m_ConsoleBuffer = "Console Begin";
		std::string m_CurrentLineBuffer = "";
		bool m_Typing = false;
	};
}

#endif