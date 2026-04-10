#include <optional>
#include <SFML/Graphics.hpp>

#include "Engine/Globals.h"
#include "Engine/Input/Mouse.h"
#include "Engine/Input/Keyboard.h"
#include "Engine/Timer.h"
#include "Game/Game.h"
#include <iostream>
#include <zlib.h>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "Engine/Asserts.h"

GraphicSettings GRAPHIC_SETTINGS{ };
RandomRangeGenerator RNG = RandomRangeGenerator(0.0, 1.0);

int main(int argc, char** argv)
{
	std::cout << "zlib version: " << zlibVersion() << "\n";

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.script("function f (a, b, c, d) print(b) return 1 end");
	lua.script("function g (a, b) return a + b end");

	// sol::function is often easier: 
	// takes a variable number/types of arguments...
	sol::function fx = lua["f"];
	// fixed signature std::function<...>
	// can be used to tie a sol::function down
	std::function<int(int, double, int, std::string)> stdfx = fx;

	int is_one = stdfx(1, 34.5, 3, "bark");
	ASSERT(is_one == 1);

	int is_also_one = fx(1, "boop", 3, "bark");
	ASSERT(is_also_one == 1);

	// call through operator[]
	int is_three = lua["g"](1, 2);
	ASSERT_MSG(is_three == 3, "is_three is NOT three!");

	double is_4_8 = lua["g"](2.4, 2.4);
	ASSERT_MSG(is_4_8 == 4.8, "2.4 + 2.4 should be 4.8!");


	sf::RenderWindow window(
#if !BUILD_MASTER
		sf::VideoMode({ 768, 576 }),
#else
		sf::VideoMode::getDesktopMode(),
#endif
		"PokéTom",
#if !BUILD_MASTER
		sf::State::Windowed
#else
		sf::State::Fullscreen
#endif
	);

	GRAPHIC_SETTINGS.SetScreenSize(window.getSize());

	window.setFramerateLimit(255);

	Mouse::Get().SetRelativeWindow(&window);

	Game game{ };

	constexpr sf::Color clearColour(0x2B2B2BFF);

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

		const float deltaTime = Timer::Get().DeltaTime();
		UPDATEABLE_REGISTRY.UpdateAll(deltaTime);

		window.clear(clearColour);

		game.Render(window);

		window.display();
	}
}
