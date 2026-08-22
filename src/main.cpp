#include <optional>
#include <SFML/Graphics.hpp>

#include "Engine/Globals.h"
#include "Engine/Input/Mouse.h"
#include "Engine/Input/Keyboard.h"
#include "Engine/Timer.h"
#include "Game/Game.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "Engine/EngineEvents.h"
#include "Engine/Language.h"
#include "Engine/Stringtable.h"
#include "Engine/Rendering/BitMapFont.h"
#include "Game/DialogueManager.h"
#include "Game/LuaRegistry.h"

GraphicSettings GRAPHIC_SETTINGS{};
RandomRangeGenerator<double> RNG = RandomRangeGenerator(0.0, 1.0);

eLanguage CHOSEN_LANGUAGE = eLanguage::English;

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

	window.setFramerateLimit(255);

	sol::state lua;
	lua.open_libraries(
		sol::lib::base,
		sol::lib::math,
		sol::lib::string,
		sol::lib::table
	);

	// Ensure that the dialogue system is all loaded
	StringTable::Get();
	DialogueManager::Get();

	// Register all the Lua bindings
	Game game{ lua };

	engine_events::OnWindowResized.Fire(window.getSize());
	Mouse::Get().SetRelativeWindow(&window);

	constexpr sf::Color clearColour(0x2B2B2BFF);

	const glyph_table_t test{
		{ 'H', { { { 219, 120 }, { 7, 16 } }, 5 } },
		{ 'E', { { { 198, 120 }, { 7, 16 } }, 5 } },
		{ 'L', { { { 246, 120 }, { 7, 16 } }, 5 } },
		{ 'O', { { { 267, 120 }, { 7, 16 } }, 5 } },
		{ 'W', { { { 322, 120 }, { 7, 16 } }, 5 } },
		{ 'R', { { { 288, 120 }, { 7, 16 } }, 5 } },
		{ 'D', { { { 191, 120 }, { 7, 16 } }, 5 } },
		{ ' ', { { { 256, 152 }, { 7, 16 } }, 5 } },
	};

	BitMapFont testString(HASH("BATTLE_HUD"), sf::IntRect{ { 171, 120 }, { 190, 64 } }, test, 16, 4);
	auto vertArray = testString.BuildText("HELLO WORLD", { 0, 0 });

	testString.SetFontSize(26.f);

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
				engine_events::OnWindowResized.Fire(window.getSize());
			}
		}

		Keyboard::Get().Update();
		Mouse::Get().Update();

		Timer::Get().Update();

		const float deltaTime = Timer::Get().DeltaTime();
		game.Update(deltaTime);

		// Update the mouse position in the potentially resized window
		UIMANAGER.Update(deltaTime, window);

		window.clear(clearColour);

		game.Render(window);

		testString.Render(window, { 462, 242 }, vertArray);

		window.display();
	}
}
