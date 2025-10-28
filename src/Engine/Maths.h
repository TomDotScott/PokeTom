#ifndef MATHS_H
#define MATHS_H
#include <SFML/System/Vector2.hpp>

namespace maths
{
	// Code adapted from Unity's implementation
	// https://stackoverflow.com/questions/61372498/how-does-mathf-smoothdamp-work-what-is-it-algorithm
	inline sf::Vector2f SmoothDamp(const sf::Vector2f& current, const sf::Vector2f& target, sf::Vector2f& velocity, const float smoothTime, const float deltaTime)
	{
		const float omega = 2.f / smoothTime;
		const float x = omega * deltaTime;
		const float exp = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);

		const sf::Vector2f delta = current - target;

		const sf::Vector2f temp = (velocity + omega * delta) * deltaTime;
		velocity = (velocity - omega * temp) * exp;

		return target + (delta + temp) * exp;
	}
} // namespace maths

#endif
