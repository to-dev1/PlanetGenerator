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

int Random::next()
{
	seed = (214013 * seed);
	return (seed >> 16) & 0x7FFF;
}