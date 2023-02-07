#pragma once
#ifndef CONSOLE_H
#define CONSOLE_H

#include "core/GUI.h"
#include "core/Root.h"
#include "string"

namespace SGRoot
{
	class ConsoleWindow : public SGGUI::GUIBase
	{
	public:
		void start(float xOff, float yOff, float screenW, float screenH);
		void end();
	};
}

#endif