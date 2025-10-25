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
#include "Game/TileSheet.h"

GraphicSettings GRAPHIC_SETTINGS{};
RandomRangeGenerator RNG = RandomRangeGenerator(0.0, 1.0);

static void SetupSprite(sf::Sprite& sprite)
{
	const sf::Vector2u textureSize = sprite.getTexture().getSize();
	const std::size_t tileCount = (textureSize.x / 32) * (textureSize.y / 32);
	const std::size_t randomTileIndex{ rand() % tileCount };

	sprite.setTextureRect({
		{
			(static_cast<int>(randomTileIndex) % 32) * 32,
			(static_cast<int>(randomTileIndex) / 32) * 32
		},
		{ 32, 32 }
		});

	sprite.setPosition({ static_cast<float>(rand() % 650), static_cast<float>(rand() % 450) });
}


int main(int argc, char** argv)
{
	std::cout << "zlib version: " << zlibVersion() << "\n";

	TileSheet sheet("tiled_export\\starter_town.tmj");

	/*SpriteBatcher batcher(std::make_shared<sf::Texture>(spriteSheetTexture));

	std::vector<sf::Sprite> sprites;
	sprites.resize(10000, sf::Sprite(spriteSheetTexture));

	std::for_each(sprites.begin(), sprites.end(), SetupSprite);

	batcher.BatchSprites(sprites);*/


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

		//window.draw(batcher);
		game.Render(window);

		window.display();
	}
}
