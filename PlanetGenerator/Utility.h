#pragma once

#include <stdint.h>

class Color
{
public:
	uint8_t r;
	uint8_t g;
	uint8_t b;

	Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) : r(red), g(green), b(blue) {};

	Color operator+(const Color& c) const;
	Color operator*(float m) const;

	unsigned int toInt() const;
};

class Vector2
{
public:
	float x;
	float y;

	Vector2(float vx = 0.0f, float vy = 0.0f) : x(vx), y(vy) {};

	float magnitudeSqr() const;
	float sqrDist(const Vector2& v) const;

	Vector2 operator+(const Vector2& v) const;
	Vector2 operator-(const Vector2& v) const;
	Vector2& operator+=(const Vector2& v);
	Vector2 operator*(float m) const;
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