#pragma once
#ifndef SGUTIL_H
#define SGUTIL_H

#include "iostream"
#include "math.h"
#include "chrono"
#include "mutex"
#include "vector"
#include "syncstream"
#include "assert.h"
#include "string"
#include "textflow/TextFlow.hpp"

static const char ENGINE_TAG[]{ "[Space Engine] " };

template<typename... args>
static void Log(args && ... inputs)
{
	std::osyncstream bout(std::cout);
	bout << ENGINE_TAG;
	([&]()	{
		bout << inputs;
	} (), ...);

	bout << "\n";
}

#if _DEBUG

#define EngineLog(vars, ...) Log(vars, ##__VA_ARGS__)
#define EngineLogOk(vars, ...) Log(vars, ##__VA_ARGS__, "[", "\033[1;32mOK\033[0m", "]")
#define EngineLogFail(vars, ...) Log(vars, ##__VA_ARGS__, "[", "\033[1;31mFAIL\033[0m", "]")

#else

#define EngineLog(vars, ...)
#define EngineLogOk(vars, ...)
#define EngineLogFail(vars, ...)

#endif

//For logging times and so is not fully secure
class EngineTimer
{
public:
	static unsigned int StartTimer();
	static double EndTimer(unsigned int id);
private:
	struct Clock
	{
		std::chrono::system_clock::time_point timeStart;
		unsigned int id;
	};
	static std::mutex s_Mutex;
	static unsigned int s_RequestId; //if by some chance reaches max value, just wrap around as it shouldn't matter
	static std::vector<Clock> m_TimerRequests;
};

//Uses TextFlow fitting to fit string to text box
std::string WrapTextbox(std::string input, int width, int pt);

//Uses TextFlow fitting to fit string to text box
std::string WrapTextbox(std::string input, int width, int pt, float fontRatio);

//Uses TextFlow to create columns of text
std::string WrapTextColumns(std::string col1, std::string col2, int width, int pt, int spacing);

//Splits string into a vector of words
bool SplitStringToWords(std::string& input, std::vector<std::string>& wordsArray);

#endif
