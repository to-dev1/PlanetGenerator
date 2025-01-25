#include "Utility.h"

Color Color::operator+(const Color& c) const
{
	return Color(r + c.r, g + c.g, b + c.b);
}

Color Color::operator*(float m) const
{
	return Color(r * m, g * m, b * m);
}

unsigned int Color::toInt() const
{
	return (static_cast<unsigned int>(r) << 16) + (static_cast<unsigned int>(g) << 8) + (static_cast<unsigned int>(b));
}

Vector2 Vector2::operator+(const Vector2& v) const
{
	return Vector2(x + v.x, y + v.y);
}

Vector2& Vector2::operator+=(const Vector2& v)
{
	x += v.x;
	y += v.y;

	return *this;
}

Vector2 Vector2::operator*(float m) const
{
	return Vector2(x * m, y * m);
}

int Random::next()
{
	seed = (214013 * seed);
	return (seed >> 16) & 0x7FFF;
}