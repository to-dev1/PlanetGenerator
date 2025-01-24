#pragma once

#include <stdint.h>

class Color
{
public:
	uint8_t r;
	uint8_t g;
	uint8_t b;

	Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) : r(red), g(green), b(blue) {};

	unsigned int toInt() const;
};

class Random
{
public:
	Random(int s) : seed(s) {};

	//Get next random value
	int next();

private:
	int seed; //Current seed
};