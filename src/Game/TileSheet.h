#ifndef TILESHEET_H
#define TILESHEET_H
#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "../Engine/TiledParsers.h"
#include "../Engine/Rendering/SpriteBatcher.h"

class TileSheet
{
public:
	struct Tile
	{
		enum Properties : uint32_t
		{
			Empty = 0,

			IsBarrier = 1 << 0,
			IsBreakable = 1 << 1,
			IsGrass = 1 << 2,
			IsWater = 1 << 3,
			IsDoor = 1 << 4,

			All = 1 << 16
		};

		uint32_t m_IDFromTMJ;
		uint32_t m_IDRelativeToParentTileSet;
		uint32_t m_Flags;
		std::string m_ParentTileSetName;
	};

	TileSheet(const std::filesystem::path& tmjPath);
	void Render(sf::RenderWindow& window) const;

	Tile GetTile(const std::string& layerName, uint32_t tileGID) const;

private:
	// TODO: We shouldn't have this many maps! Needs a lot of cleanup!
	std::unordered_map<uint32_t, uint32_t> m_idMappings;

	// < KEY = LAYER NAME, VALUE = < KEY = OffsetTileGID, VALUE = TileInfo > >
	std::unordered_map<std::string, std::unordered_map<uint32_t, Tile>> m_layers;
	std::unordered_map<std::string, std::shared_ptr<TSX>> m_tileSets;
	std::shared_ptr<TMJ> m_tmjParser;


	// TODO: Probably shouldn't be loaded here, but from the TSX parser itself
	// TODO: Integrate with the TextureManager class
	std::unordered_map<std::string, std::shared_ptr<sf::Texture>> m_spriteSheets;

	std::unordered_map<std::string, std::vector<sf::Sprite>> m_tileSetSprites;
	std::unordered_map<std::string, SpriteBatcher> m_tileSetSpriteBatchers;
};

#endif
