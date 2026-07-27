#include "Globals.h"

#include <SFML/Graphics/Font.hpp>

#include "EngineEvents.h"

GraphicSettings::GraphicSettings() :
	m_screenDetails(),
	m_letterboxViewport({}, { 1.f, 1.f })
{
	// Not sure if I like relying on this being constructed first, maybe it should fire the event?
	engine_events::OnWindowResized.On([this](const sf::Vector2u newSize) { SetScreenSize(newSize); });
}

const GraphicSettings::ScreenDetails& GraphicSettings::GetScreenDetails() const
{
	return m_screenDetails;
}

GraphicSettings::ScreenDetails GraphicSettings::GetScreenDetails()
{
	return m_screenDetails;
}

sf::FloatRect GraphicSettings::GetLetterboxViewport() const
{
	return m_letterboxViewport;
}

void GraphicSettings::SetScreenSize(const sf::Vector2u screenSize)
{
	m_screenDetails = {
		.m_ScreenSize = screenSize,
		.m_ScreenCentre = screenSize / 2u
	};

	m_letterboxViewport = ComputeLetterboxViewport(screenSize);
}

sf::FloatRect GraphicSettings::ComputeLetterboxViewport(const sf::Vector2u newScreenSize)
{
	static constexpr float k_referenceAspectRatio = static_cast<float>(REFERENCE_SCREEN_SIZE.x) / static_cast<float>(
		REFERENCE_SCREEN_SIZE.y);

	const float newAspectRatio = static_cast<float>(newScreenSize.x) / static_cast<float>(newScreenSize.y);

	if (newAspectRatio > k_referenceAspectRatio)
	{
		const float width = k_referenceAspectRatio / newAspectRatio;
		return { { (1.f - width) / 2.f, 0.f }, { width, 1.f } };
	}

	const float height = newAspectRatio / k_referenceAspectRatio;
	return { { 0.f, (1.f - height) / 2.f }, { 1.f, height } };
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
