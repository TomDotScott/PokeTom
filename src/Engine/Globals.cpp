#include "Globals.h"

#include <SFML/Graphics/Font.hpp>

RandomRangeGenerator::RandomRangeGenerator(const double min, const double max) :
	m_dist(min, max), m_randomEngine(m_randomDevice())
{
}

bool OnlyWhitespace(const char* chr)
{
	while (*chr != '\0')
	{
		if (*chr != ' ' && *chr != '\t' && *chr != '\n' && *chr != '\r')
		{
			return false;
		}
		chr++;
	}
	return true;
}

sf::Font DEFAULT_FONT("fonts/FiraCode-Regular.ttf");

