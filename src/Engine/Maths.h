#ifndef MATHS_H
#define MATHS_H
#include <type_traits>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

namespace maths
{
	// Code adapted from Unity's implementation
	// https://stackoverflow.com/questions/61372498/how-does-mathf-smoothdamp-work-what-is-it-algorithm
	inline sf::Vector2f SmoothDamp(const sf::Vector2f& current, const sf::Vector2f& target, sf::Vector2f& velocity,
	                               const float smoothTime, const float deltaTime)
	{
		const float omega = 2.f / smoothTime;
		const float x = omega * deltaTime;
		const float exp = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);

		const sf::Vector2f delta = current - target;

		const sf::Vector2f temp = (velocity + omega * delta) * deltaTime;
		velocity = (velocity - omega * temp) * exp;

		return target + (delta + temp) * exp;
	}


	template<typename T>
	T Lerp(const T a, const T b, const float t)
	{
		return a + t * (b - a);
	}

	inline sf::Color Lerp(const sf::Color a, const sf::Color b, const float t)
	{
		return {
			static_cast<uint8_t>(Lerp(static_cast<float>(a.r), static_cast<float>(b.r), t)),
			static_cast<uint8_t>(Lerp(static_cast<float>(a.g), static_cast<float>(b.g), t)),
			static_cast<uint8_t>(Lerp(static_cast<float>(a.b), static_cast<float>(b.b), t))
		};
	}
	

	template <typename T>
	T Clamp(T val, T min, T max)
		requires std::is_arithmetic_v<T>
	{
		if (val < min)
		{
			return min;
		}
		if (val > max)
		{
			return max;
		}

		return val;
	}
} // namespace maths

#endif
