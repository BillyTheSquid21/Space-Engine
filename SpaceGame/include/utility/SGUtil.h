#pragma once
#ifndef SGUTIL_H
#define SGUTIL_H

#include <iostream>
#include <math.h>
#include <chrono>
#include <mutex>
#include <vector>
#include <syncstream>

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

#else

#define EngineLog(vars, ...) (0 && Log(vars, ##__VA_ARGS__))

#endif

float invSqrt(float number); //Supposedly a faster inv sqrt
int intFloor(float x); //supposedly faster floor
int roundDownMultiple(float num, int multiple);
int roundUpMultiple(float num, int multiple);

//return structs
struct Struct2f
{
	float a, b;
};

struct Struct3f
{
	float a, b, c;
};

struct Struct4f
{
	float a, b, c, d;
};

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

#endif
