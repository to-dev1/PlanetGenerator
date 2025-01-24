#include "Utility.h"

unsigned int Color::toInt() const
{
	return (static_cast<unsigned int>(r) << 16) + (static_cast<unsigned int>(g) << 8) + (static_cast<unsigned int>(b));
}

int Random::next()
{
	seed = (214013 * seed);
	return (seed >> 16) & 0x7FFF;
}