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

std::string WrapTextbox(std::string input, int width, int pt)
{
	//Assume char w is 0.55*pt size
	return WrapTextbox(input, width, pt, 0.55f);
}

std::string WrapTextbox(std::string input, int width, int pt, float fontRatio)
{
	//Assume char w is 0.65*pt size
	auto txt = TextFlow::Column(input).width(width / (fontRatio * pt));
	return txt.toString();
}

std::string WrapTextColumns(std::string col1, std::string col2, int width, int pt, int spacing)
{
	using namespace TextFlow;
	size_t colWidth = ((size_t)width / 2u) - (size_t)spacing;
	auto l = Column(col1).width(colWidth);
	auto r = Column(col2).width(colWidth);
	auto layout = l + Spacer(spacing) + r;
	return layout.toString();
}

bool SplitStringToWords(std::string& input, std::vector<std::string>& wordsArray)
{
	//Move along string, finding next instance of " " or "\n" or at string.length()
	//Then extract substring to that point, add to array
	//If string is empty or invalid, return false

	//Check cases where string is 1 letter or none
	if (input.length() == 0)
	{
		return false;
	}

	if (input.length() == 1)
	{
		wordsArray.push_back(input);
		return true;
	}

	size_t lastStartIndex = 0;
	for (size_t i = 0; i < input.length() - 1u; i++)
	{
		char c = input[i + 1u]; //Find next char
		size_t size = 0;
		if (c == ' ' || c == '\n')
		{
			size = (size_t)(i - lastStartIndex + 1u);
		}
		else if (i + 2u == input.length())
		{
			size = (size_t)(i - lastStartIndex + 2u); //Need to go further if end
		}

		//If a word end was found push word back
		if (size > 0)
		{
			wordsArray.push_back(input.substr(lastStartIndex, size));
			lastStartIndex = i + 2u;
		}
	}
	return true;
}

