#include "TileSheet.h"

#include <fstream>
#include <iostream>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>


TileSheet::TileSheet(const std::filesystem::path& tmjPath) :
	m_tmjParser(TMJ::Create(tmjPath)),
	m_spriteBatcher(nullptr)
{
	if (m_tmjParser == nullptr)
	{
		std::cout << "Error: Failed to load TMJ file " << tmjPath << "\n";
	}

	// Using the tilesets from the TMJ file, parse the XML TSX file to load the sprite info
	const std::vector<TMJ::TileSet>& tileSets = m_tmjParser->GetTileSets();
	for (const auto& tileSet : tileSets)
	{
		const auto& tsxParser = TSX::Create(tileSet.m_TsxSource);
		const std::string& tileSetName = tsxParser->GetTileSetInfo().m_Name;
		if (m_tileSets.find(tileSetName) == m_tileSets.end())
		{
			m_tileSets[tileSetName] = tsxParser;
		}
	}

	// Then, build up the tile contents for this level
	// Iterate over each layer from the TMJ, getting the correct IDs for the tiles
	for (const auto& layer : m_tmjParser->GetLayers())
	{
		// TODO: Width, height and all the other crap...
		for (const auto& tileGID : layer.m_Data)
		{
			// Work out which tileset it belongs to
			std::string parentTileSetName;
			uint32_t gidOffset = 0;
			bool found = false;

			for (const auto& [firstGid, tsxSource] : tileSets)
			{
				int offset = static_cast<int>(tileGID) - firstGid + 1;
				if (offset >= 0) {
					for (const auto& [tileSetName, tsxTileSet] : m_tileSets)
					{
						if (tsxSource == tsxTileSet->GetPath())
						{
							parentTileSetName = tileSetName;
							gidOffset = firstGid;
							found = true;
						}

						if (found)
						{
							break;
						}
					}
				}

				if (found)
				{
					break;
				}
			}

			if (!found)
			{
				std::cout << "Error: Cannot locate tileset for tile=" << tileGID << " in layer " << layer.m_Name <<
					"of sheet " << tmjPath << "\n";
				break;
			}

			const auto& parentTileSet = m_tileSets.at(parentTileSetName);

			// -1 means empty, so we don't want integer overflow!
			int tileIndex = static_cast<int>(tileGID) - gidOffset;
			if (tileIndex == -1)
			{
				continue;
			}

			if (m_layers.find(layer.m_Name) == m_layers.end())
			{
				m_layers[layer.m_Name] = {};
			}

			const auto& currentLayer = m_layers.at(layer.m_Name);
			if (currentLayer.find(tileIndex) != currentLayer.end())
			{
				// We already have the info ready for this tile
				continue;
			}

			uint32_t tileFlags = 0;
			const auto& tileSetTiles = parentTileSet->GetTiles();

			auto foundTile = std::find_if(tileSetTiles.begin(), tileSetTiles.end(), [&](const TSX::Tile& a)
				{
					return a.m_ID == tileIndex;
				});

			if (foundTile == tileSetTiles.end())
			{
				std::cout << "Failed to find tile with ID " << tileIndex << " in layer " << layer.m_Name << "\n";
				continue;
			}

			const TSX::Tile& tsxTile = *foundTile;
			for (const auto& tileProperties : tsxTile.m_Properties)
			{
				// TODO: These probably shouldn't be hardcoded!
				if (tileProperties.m_Name == "isBarrier")
				{
					tileFlags |= Tile::Properties::IsBarrier;
				}
				else if (tileProperties.m_Name == "isBreakable")
				{
					tileFlags |= Tile::Properties::IsBreakable;
				}
				else if (tileProperties.m_Name == "isGrass")
				{
					tileFlags |= Tile::Properties::IsGrass;
				}
				else if (tileProperties.m_Name == "isWater")
				{
					tileFlags |= Tile::Properties::IsWater;
				}
				else if (tileProperties.m_Name == "isDoor")
				{
					tileFlags |= Tile::Properties::IsDoor;
				}
			}

			const Tile gameTile{ tsxTile.m_ID, tileFlags, parentTileSetName };
			m_layers[layer.m_Name][tileGID] = gameTile;
		}
	}



	/*const auto& tileSetsArray = tilesheetJSON["tilesets"];
	std::vector<TileSet> tileSets;
	if (tileSetsArray.type() == nlohmann::detail::value_t::array)
	{
		for (const auto& elem : tileSetsArray)
		{
			const TileSet tileSet{ elem["firstgid"], elem["source"] };
			tileSets.emplace_back(tileSet);
		}
	}

	for (const auto& [layerName, tileGIDs] : layers)
	{
		m_layers[layerName] = std::vector<Tile>(layers.at(layerName).size());

		for (const uint32_t gid : tileGIDs)
		{
		}
	}

	if (!SetUpSpriteBatcher())
	{
		std::cout << "Failed to set up SpriteBatcher" << "\n";
	}


	m_spriteBatcher.BatchSprites()*/
}

void TileSheet::Render(sf::RenderWindow& window) const
{
	window.draw(m_spriteBatcher);
}

bool TileSheet::SetUpSpriteBatcher()
{
	sf::Image spriteSheet;
	if (!spriteSheet.loadFromFile("sprites\\overworld_tileset.png"))
	{
		std::cout << "ERROR LOADING SPRITESHEET TEXTURE FROM sf::Image!" << "\n";
		return false;
	}

	spriteSheet.createMaskFromColor(sf::Color(0, 0, 0, 0));

	/*m_spriteSheetTexture = std::make_shared<sf::Texture>(sf::Texture());
	if (!m_spriteSheetTexture->loadFromImage(spriteSheet))
	{
		std::cout << "ERROR LOADING SPRITESHEET TEXTURE FROM sf::Image!" << "\n";
		return false;
	}

	m_spriteBatcher = SpriteBatcher(m_spriteSheetTexture);*/

	return true;
}
