#ifndef ENGINEEVENTS_H
#define ENGINEEVENTS_H
#include <SFML/System/Vector2.hpp>

#include "Event.h"

namespace engine_events
{
	inline auto OnWindowResized = Event<sf::Vector2u>();
}

#endif // ENGINEEVENTS_H
