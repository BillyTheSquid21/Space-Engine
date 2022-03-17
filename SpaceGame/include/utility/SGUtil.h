#pragma once
#ifndef SGUTIL_H
#define SGUTIL_H

#include <iostream>
#include <math.h>

static const char ENGINE_TAG[]{ "[Space Engine] " };

template<typename T>
void EngineLog(T value) {
	std::cout << ENGINE_TAG;
	std::cout << value << "\n";
}
template<typename T>
void EngineLog(std::string message,T value) {
	std::cout << ENGINE_TAG;
	std::cout << message;
	std::cout << value << "\n";
}

template<typename T>
void EngineLog(T value1, T value2) {
	std::cout << ENGINE_TAG;
	std::cout << value1 << " ";
	std::cout << value2 << "\n";
}

template<typename T>
void EngineLog(std::string message, T value1, T value2) {
	std::cout << ENGINE_TAG;
	std::cout << message << " ";
	std::cout << value1 << " ";
	std::cout << value2 << "\n";
}

template<typename T>
void EngineLog(std::string message, T value1, T value2, T value3) {
	std::cout << ENGINE_TAG;
	std::cout << message << " ";
	std::cout << value1 << " ";
	std::cout << value2 << " ";
	std::cout << value3 << "\n";
}

float invSqrt(float number); //Supposedly a faster inv sqrt
int intFloor(float x); //supposedly faster floor
int roundDownMultiple(float num, int multiple);
int roundUpMultiple(float num, int multiple);

//return structs
struct Component2f
{
	float a, b;
};

struct Component3f
{
	float a, b, c;
};

struct Component4f
{
	float a, b, c, d;
};

#endif
