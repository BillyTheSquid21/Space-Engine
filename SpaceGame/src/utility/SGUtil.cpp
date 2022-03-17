#include "utility/SGUtil.h"

float invSqrt(float number) {
	float squareRoot = sqrt(number);
	return 1.0f / squareRoot;
}

int intFloor(float x)
{
	int i = (int)x; /* truncate */
	return i - (i > x); /* convert trunc to floor */
}

int roundDownMultiple(float num, int multiple) 
{
	int numInt = (int)num;

	// Smaller multiple
	int a = (numInt / multiple) * multiple;

	// Larger multiple
	int b = a + multiple;

	// Return of closest of two
	if (numInt - a > b - numInt) {
		return b;
	}
	return a;
}

int roundUpMultiple(float num, int multiple)
{
	int numInt = (int)num;

	// Smaller multiple
	int a = (numInt / multiple) * multiple;

	// Larger multiple
	int b = a + multiple;

	// Return of closest of two
	if (numInt - a > b - numInt) {
		return a;
	}
	return b;
}

