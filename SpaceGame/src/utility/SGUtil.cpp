#include "utility/SGUtil.h"

//timer
std::vector<EngineTimer::Clock> EngineTimer::m_TimerRequests;
unsigned int EngineTimer::s_RequestId = 0;
std::mutex EngineTimer::s_Mutex;

unsigned int EngineTimer::StartTimer()
{
	std::lock_guard<std::mutex> lock(s_Mutex);
	unsigned int id = s_RequestId;
	EngineTimer::Clock clock = { std::chrono::system_clock::now(), id };
	m_TimerRequests.emplace_back(clock);
	s_RequestId++;
	return id;
}

double EngineTimer::EndTimer(unsigned int id)
{
	for (int i = 0; i < m_TimerRequests.size(); i++)
	{
		if (id == m_TimerRequests[i].id)
		{
			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsedSeconds = end - m_TimerRequests[i].timeStart;
			std::lock_guard<std::mutex> lock(s_Mutex);
			m_TimerRequests.erase(m_TimerRequests.begin() + i);
			return elapsedSeconds.count();
		}
	}
	return -1.0;
}

