#include <optional>
#include <SFML/Graphics.hpp>

#include "Engine/Globals.h"
#include "Engine/Input/Mouse.h"
#include "Engine/Input/Keyboard.h"
#include "Engine/Timer.h"
#include "Game/Game.h"

GraphicSettings GRAPHIC_SETTINGS{};
RandomRangeGenerator RNG = RandomRangeGenerator(0.0, 1.0);

int main(int argc, char** argv)
{
	sf::RenderWindow window(
#if BUILD_DEBUG
		sf::VideoMode({ 768, 576 }),
#else
		sf::VideoMode::getDesktopMode(),
#endif
		"PokéTom",
#if BUILD_DEBUG
		sf::State::Windowed
#else
		sf::State::Fullscreen
#endif
	);

	GRAPHIC_SETTINGS.SetScreenSize(window.getSize());

	window.setFramerateLimit(60);

	Mouse::Get().SetRelativeWindow(&window);

	Game game{};

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
			if (event->is<sf::Event::Resized>())
			{
				GRAPHIC_SETTINGS.SetScreenSize(window.getSize());
			}
		}

		Keyboard::Get().Update();
		Mouse::Get().Update();

		Timer::Get().Update();

		game.Update();

		window.clear();

		game.Render(window);

		window.display();
	}
}

