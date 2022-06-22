#pragma once
#ifndef SG_RANDOM_H
#define SG_RANDOM_H

#include <random>

//container for basic random
class RandomContainer
{
public:
	void seed(float a, float b) { std::random_device rd;  mt.seed(rd()); dist = std::uniform_real_distribution<float>::uniform_real_distribution(a, b); m_Seeded = true; }
	float next() { return dist(mt); }
	bool isSeeded() const { return m_Seeded; }
private:
	bool m_Seeded = false;
	std::mt19937 mt;
	std::uniform_real_distribution<float> dist;
};

#endif