#include "TileSheet.h"

#include <fstream>
#include <iostream>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "../Engine/Globals.h"


TileSheet::TileSheet(const std::filesystem::path& tmjPath) :
	m_tmjParser(TMJ::Create(tmjPath))
{
	// TODO: This constructor is gonna be a nightmare to debug if I don't clean it up
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
			m_spriteSheets[tileSetName] = std::make_shared<sf::Texture>(tsxParser->GetImageInfo().m_Source);
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
				if (offset >= 0)
				{
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
			const int offsetTileIndexFromFirstGID = static_cast<int>(tileGID) - static_cast<int>(gidOffset);
			if (offsetTileIndexFromFirstGID == -1)
			{
				continue;
			}

			m_idMappings[tileGID] = offsetTileIndexFromFirstGID;

			if (m_layers.find(layer.m_Name) == m_layers.end())
			{
				m_layers[layer.m_Name] = {};
			}

			const auto& currentLayer = m_layers.at(layer.m_Name);
			if (currentLayer.find(offsetTileIndexFromFirstGID) != currentLayer.end())
			{
				// We already have the info ready for this tile
				continue;
			}

			uint32_t tileFlags = 0;
			const auto& tileSetTiles = parentTileSet->GetTiles();

			auto foundTile = std::find_if(tileSetTiles.begin(), tileSetTiles.end(), [&](const TSX::Tile& a)
				{
					return a.m_ID == offsetTileIndexFromFirstGID;
				});

			if (foundTile == tileSetTiles.end())
			{
				std::cout << "Failed to find tile with ID " << offsetTileIndexFromFirstGID << " in layer " << layer.
					m_Name << "\n";
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

			const Tile gameTile{
				tileGID,
				static_cast<uint32_t>(offsetTileIndexFromFirstGID),
				tileFlags,
				parentTileSetName
			};

			m_layers[layer.m_Name][offsetTileIndexFromFirstGID] = gameTile;
		}
	}

	// We now have the tile data set up! Create the sprite objects for them
	const sf::Vector2u screenSize = GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize;

	for (const auto& layer : m_tmjParser->GetLayers())
	{
		// TODO: Fix this!
		if (layer.m_Name == "NPC Spawns")
		{
			continue;
		}

		for (int i = 0; i < layer.m_Data.size(); ++i)
		{
			const uint32_t offsetTileID = layer.m_Data[i];
			if (offsetTileID == 0)
			{
				continue;
			}

			Tile currentTile = GetTile(layer.m_Name, offsetTileID);
			const auto& parentTileSet = m_tileSets.at(currentTile.m_ParentTileSetName);

			std::shared_ptr<sf::Texture> tileSheet = m_spriteSheets.at(currentTile.m_ParentTileSetName);
			if (m_tileSetSpriteBatchers.find(currentTile.m_ParentTileSetName) == m_tileSetSpriteBatchers.end())
			{
				m_tileSetSpriteBatchers[currentTile.m_ParentTileSetName] = SpriteBatcher(tileSheet);
			}

			sf::Sprite sprite(*tileSheet);

			int tileWidth = parentTileSet->GetTileSetInfo().m_TileWidth;
			int tileHeight = parentTileSet->GetTileSetInfo().m_TileHeight;
			int tilesPerRow = parentTileSet->GetTileSetInfo().m_NumColumns;

			int localId = currentTile.m_IDRelativeToParentTileSet; // should already be 0-based
			int x = (localId % tilesPerRow) * tileWidth;
			int y = (localId / tilesPerRow) * tileHeight;

			sprite.setTextureRect({ { x, y }, { tileWidth, tileHeight } });
			sprite.setScale({ 0.75, 0.75 });

			// Work out what position the sprite is in on screen
			const uint32_t numCols = m_tmjParser->GetNumColumns();
			const uint32_t numRows = m_tmjParser->GetNumRows();

			const sf::Vector2f spritePosition = sf::Vector2f({
				static_cast<float>((i % numCols) * /*16*/24),
				static_cast<float>((i / numCols) * /*16*/24)
				});

			sprite.setPosition(spritePosition);

			m_tileSetSprites[currentTile.m_ParentTileSetName].emplace_back(sprite);
		}
	}

	for (const auto& [tileSetName, spritesArray] : m_tileSetSprites)
	{
		m_tileSetSpriteBatchers.at(tileSetName).BatchSprites(spritesArray);
	}
}


void TileSheet::Render(sf::RenderWindow& window) const
{
	for (const auto& [tileSetName, batcher] : m_tileSetSpriteBatchers) {
		window.draw(batcher);
	}
}

TileSheet::Tile TileSheet::GetTile(const std::string& layerName, const uint32_t tileGID) const
{
	const uint32_t offSetID = m_idMappings.at(tileGID);
	return m_layers.at(layerName).at(offSetID);
}
