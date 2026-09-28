#ifndef GLOBALS_H
#define GLOBALS_H
#include <random>
#include <span>
#include <sstream>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#define RENDER_SPRITES 1

namespace sf
{
	class Font;
}

class GraphicSettings
{
public:
	GraphicSettings();

	struct ScreenDetails
	{
		sf::Vector2u m_ScreenSize;
		sf::Vector2u m_ScreenCentre;
	};

	const ScreenDetails& GetScreenDetails() const;
	ScreenDetails GetScreenDetails();
	sf::FloatRect GetLetterboxViewport() const;

private:
	ScreenDetails m_screenDetails;
	sf::FloatRect m_letterboxViewport;

	void SetScreenSize(sf::Vector2u screenSize);
	sf::FloatRect ComputeLetterboxViewport(sf::Vector2u newScreenSize);
};


template<typename T, typename TDist = std::conditional_t<std::is_floating_point_v<T>, std::uniform_real_distribution<T>, std::uniform_int_distribution<T>>>
class RandomRangeGenerator
{
public:
	RandomRangeGenerator(const T min, const T max) :
		m_dist(min, max), m_randomEngine(m_randomDevice())
	{
	}

	T Next() { return m_dist(m_randomEngine); }

private:
	TDist m_dist;
	std::random_device m_randomDevice;
	std::default_random_engine m_randomEngine;
};

constexpr float PI = 3.141592f;

struct Vector2HashFunction
{
	size_t operator()(const sf::Vector2f& vec) const
	{
		const size_t xHash = std::hash<float>()(vec.x);
		const size_t yHash = std::hash<float>()(vec.y);
		return xHash ^ yHash;
	}
};

extern bool OnlyWhitespace(const char* chr);

#define VECTOR2F_ZERO sf::Vector2f(0.f, 0.f)
#define VECTOR2F_LEFT sf::Vector2f(-1.f, 0.f)
#define VECTOR2F_RIGHT sf::Vector2f(1.f, 0.f)
#define VECTOR2F_UP sf::Vector2f(0.f, -1.f)
#define VECTOR2F_DOWN sf::Vector2f(0.f, 1.f)

// I don't like how this is needed for sf::Text to compile
extern sf::Font DEFAULT_FONT;

constexpr static sf::Vector2u REFERENCE_SCREEN_SIZE{ 768, 576 };
extern GraphicSettings GRAPHIC_SETTINGS;
extern RandomRangeGenerator<double> RNG;

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

#endif
