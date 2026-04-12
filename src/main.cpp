#include <optional>
#include <SFML/Graphics.hpp>

#include "Engine/Globals.h"
#include "Engine/Input/Mouse.h"
#include "Engine/Input/Keyboard.h"
#include "Engine/Timer.h"
#include "Game/Game.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "Engine/Scripting/LuaRegistry.h"

GraphicSettings GRAPHIC_SETTINGS{ };
RandomRangeGenerator RNG = RandomRangeGenerator(0.0, 1.0);

int main(int argc, char** argv)
{
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

	EntityRegistry entityRegistry;

	sol::state lua;
	lua.open_libraries(
		sol::lib::base,
		sol::lib::math,
		sol::lib::string,
		sol::lib::table
	);

	// Register all the Lua bindings
	LuaRegistry luaRegistry{ lua, entityRegistry };

	Game game{ lua, entityRegistry };

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
		game.Update(deltaTime);

		window.clear(clearColour);

		game.Render(window);

		window.display();
	}
}
