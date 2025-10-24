#include <optional>
#include <SFML/Graphics.hpp>

#include "Engine/Globals.h"
#include "Engine/Input/Mouse.h"
#include "Engine/Input/Keyboard.h"
#include "Engine/Timer.h"
#include "Game/Game.h"
#include <fstream>
#include <iostream>
#include "Libs/nlohmann/json.hpp"
#include "Libs/renenyffenegger/base64.h"
#include <zlib.h>
#include <stdexcept>

static std::string DecompressZlib(const std::string& compressed) {
	z_stream stream{};
	stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed.data()));
	stream.avail_in = compressed.size();

	if (inflateInit(&stream) != Z_OK)
		throw std::runtime_error("inflateInit failed");

	std::string out;
	char buffer[32768];
	int status;
	do {
		stream.next_out = reinterpret_cast<Bytef*>(buffer);
		stream.avail_out = sizeof(buffer);
		status = inflate(&stream, Z_NO_FLUSH);
		if (status != Z_OK && status != Z_STREAM_END) {
			inflateEnd(&stream);
			throw std::runtime_error("inflate failed");
		}
		out.append(buffer, sizeof(buffer) - stream.avail_out);
	} while (status != Z_STREAM_END);

	inflateEnd(&stream);
	return out;
}

static std::vector<uint32_t> BytesToTiles(const std::string& bytes) {
	size_t count = bytes.size() / 4;
	std::vector<uint32_t> tiles(count);

	// interpret every 4 bytes as an unsigned 32-bit int (little-endian)
	for (size_t i = 0; i < count; ++i) {
		const auto b = reinterpret_cast<const unsigned char*>(bytes.data() + i * 4);
		tiles[i] =
			static_cast<uint32_t>(b[0]) |
			static_cast<uint32_t>(b[1]) << 8 |
			static_cast<uint32_t>(b[2]) << 16 |
			static_cast<uint32_t>(b[3]) << 24;

		// The compressed version adds 1 for some reason!
		tiles[i]--;
	}

	return tiles;
}


GraphicSettings GRAPHIC_SETTINGS{};
RandomRangeGenerator RNG = RandomRangeGenerator(0.0, 1.0);

int main(int argc, char** argv)
{
	std::cout << "zlib version: " << zlibVersion() << "\n";

	std::ifstream f("tiled_export\\starter_town.tmj");
	nlohmann::json data = nlohmann::json::parse(f);

	std::cout << "CompressionLevel=" << data["compressionlevel"] << "\n";
	std::cout << "height=" << data["height"] << "\n";
	std::cout << "infinite=" << data["infinite"] << "\n";

	const auto& layersArray = data["layers"];

	std::vector<std::vector<uint32_t>> layers;
	if (layersArray.type() == nlohmann::detail::value_t::array)
	{
		for (auto i = layersArray.begin(); i != layersArray.end(); ++i)
		{
			const auto& elem = *i;

			const std::string& content = elem["data"];
			const auto decodedString = base64_decode(content);
			const auto decompressed = DecompressZlib(decodedString);
			const auto tiles = BytesToTiles(decompressed);

			layers.emplace_back(tiles);
		}
	}


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
