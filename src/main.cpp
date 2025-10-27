#include <optional>
#include <SFML/Graphics.hpp>

#include "Engine/Globals.h"
#include "Engine/Input/Mouse.h"
#include "Engine/Input/Keyboard.h"
#include "Engine/Timer.h"
#include "Game/Game.h"
#include <fstream>
#include <iostream>
#include <zlib.h>
#include <stdexcept>

#include "Engine/Rendering/SpriteBatcher.h"
#include "Game/TileRenderer.h"
#include "Game/TileLogic.h"
#include "Game/TileParser.h"
#include "Game/TileSheet.h"

GraphicSettings GRAPHIC_SETTINGS{};
RandomRangeGenerator RNG = RandomRangeGenerator(0.0, 1.0);

int main(int argc, char** argv)
{
	std::cout << "zlib version: " << zlibVersion() << "\n";

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

	// window.setFramerateLimit(60);

	Mouse::Get().SetRelativeWindow(&window);

	Game game{};

	const TileMapData mapData = TileParser::ParseTMJ("tiled_export\\starter_town.tmj");
	const TileLogic logic(mapData);
	TileRenderer renderer;

	auto renderData = logic.BuildRenderData();
	renderer.BuildBatches(renderData, mapData.m_Layers);

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



		renderer.Render(window);

		game.Render(window);

		window.display();
	}
}
