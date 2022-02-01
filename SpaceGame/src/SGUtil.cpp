#include "SGUtil.h"

float invSqrt(float number) {
	float squareRoot = sqrt(number);
	return 1.0f / squareRoot;
}

int intFloor(float x)
{
	int i = (int)x; /* truncate */
	return i - (i > x); /* convert trunc to floor */
}

