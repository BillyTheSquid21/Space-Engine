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

std::string EstimateStringFit(std::string input, int boxWidth, int pt)
{
	float CHAR_WIDTH = 0.5f * (float)pt;
	int CHAR_COUNT_PER_LINE = boxWidth / (int)CHAR_WIDTH;

	std::string output = "";

	//Find white space before max count per line, add to output with \n
	//Keep going per line
	int currentChar = 0;
	int lastChar = 0;
	
	//Ensure string doesn't go over, otherwise don't need to perform any operations
	if (input.length() < CHAR_COUNT_PER_LINE)
	{
		return input;
	}

	currentChar = CHAR_COUNT_PER_LINE;
	while (lastChar < input.length())
	{
		bool fits = false;
		for (int i = currentChar; i >= lastChar; i--)
		{
			if (i < input.length() - 1 && input[i] == ' ')
			{
				//Extract the string between last location and before white space
				output += input.substr(lastChar, i - lastChar) + "\n";
				lastChar = i + 1; //Start next string after white space
				currentChar = i + CHAR_COUNT_PER_LINE;

				//If the currentChar is seeking past end, split rest of string
				//and return
				if (currentChar >= input.length())
				{
					output += input.substr(lastChar, input.length() - currentChar);
					return output;
				}
				fits = true;
				break;
			}
		}
		if (!fits)
		{
			//If no whole word will fit, just extract max
			output += input.substr(lastChar, CHAR_COUNT_PER_LINE) + "\n";
			lastChar += CHAR_COUNT_PER_LINE; //Start next string after white space
			currentChar = lastChar + CHAR_COUNT_PER_LINE;
		}
	}

	return output;
}

