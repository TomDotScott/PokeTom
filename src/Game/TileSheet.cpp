#include "TileSheet.h"

#include <fstream>
#include <iostream>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "../Engine/Globals.h"

TileSheet::TileSheet(const std::filesystem::path& tmjPath) :
	m_tmjParser(TMJ::Create(tmjPath))
{
	Init();
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

bool TileSheet::Init()
{
	if (!LoadTMJ())
	{
		return false;
	}

	if (!LoadTileSets())
	{
		return false;
	}

	BuildTileData();
	BuildSprites();
	return true;
}

bool TileSheet::LoadTMJ() const
{
	if (!m_tmjParser)
	{
		std::cout << "TileSheet::LoadTMJ: Failed to load TMJ file!\n";
		return false;
	}

	return true;
}

bool TileSheet::LoadTileSets()
{
	// Using the tilesets from the TMJ file, parse the XML TSX file to load the sprite info
	const std::vector<TMJ::TileSet>& tileSets = m_tmjParser->GetTileSets();
	for (const auto& tileSet : tileSets)
	{
		const auto& tsxParser = TSX::Create(tileSet.m_TsxSource);
		const std::string& tileSetName = tsxParser->GetTileSetInfo().m_Name;
		if (m_tileSets.find(tileSetName) == m_tileSets.end())
		{
			m_tileSets[tileSetName] = tsxParser;

			const auto& imageSource = tsxParser->GetImageInfo().m_Source;
			if (!std::filesystem::exists(imageSource))
			{
				std::cout << "TileSheet::LoadTileSets: Failed to load spritesheet from " << imageSource << "\n";
				return false;
			}

			m_spriteSheets[tileSetName] = std::make_shared<sf::Texture>(imageSource);
		}
	}

	return true;
}

void TileSheet::BuildTileData()
{
	// Then, build up the tile contents for this level
	// Iterate over each layer from the TMJ, getting the correct IDs for the tiles
	for (const auto& layer : m_tmjParser->GetLayers())
	{
		// TODO: Fix this!
		if (layer.m_Name == "NPC Spawns")
		{
			continue;
		}

		// TODO: Width, height and all the other crap...
		for (const auto& globalID : layer.m_Data)
		{
			const auto& [parentTileSetName, firstGID] = FindParentTileset(globalID);

			if (parentTileSetName.empty())
			{
				std::cout << "Error: Cannot locate tileset for tile=" << globalID << " in layer " << layer.m_Name << "\n";
				break;
			}

			const auto& parentTileSet = m_tileSets.at(parentTileSetName);

			// -1 means empty, so we don't want integer overflow!
			const int localID = static_cast<int>(globalID) - static_cast<int>(firstGID);
			if (localID < 0)
			{
				continue;
			}

			m_idMappings[globalID] = localID;

			if (m_layers.find(layer.m_Name) == m_layers.end())
			{
				m_layers[layer.m_Name] = {};
			}

			auto& currentLayer = m_layers.at(layer.m_Name);
			if (currentLayer.find(localID) == currentLayer.end())
			{
				currentLayer[localID] = CreateTileFromTSX(parentTileSet, globalID, localID);
			}
		}
	}
}

void TileSheet::BuildSprites()
{
	// We now have the tile data set up! Create the sprite objects for them
	const uint32_t numCols = m_tmjParser->GetNumColumns();

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

			int localId = currentTile.m_LocalID; // should already be 0-based
			int x = (localId % tilesPerRow) * tileWidth;
			int y = (localId / tilesPerRow) * tileHeight;

			sprite.setTextureRect({ { x, y }, { tileWidth, tileHeight } });
			sprite.setScale({ 0.75, 0.75 });

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

TileSheet::Tile TileSheet::CreateTileFromTSX(const std::shared_ptr<TSX>& parentTileset, uint32_t globalID,
	uint32_t localID)
{
	uint32_t tileFlags = Tile::Properties::None;
	const auto& tileSetTiles = parentTileset->GetTiles();

	auto foundTile = std::find_if(tileSetTiles.begin(), tileSetTiles.end(), [&](const TSX::Tile& a)
		{
			return a.m_ID == localID;
		});

	if (foundTile == tileSetTiles.end())
	{
		std::cout << "Failed to find tile with ID " << localID << " in Tileset " << parentTileset->GetTileSetInfo().m_Name << "\n";
		// TODO: We want to return a failed value and then catch it
		// throw;
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

	return {
		globalID,
		localID,
		tileFlags,
		parentTileset->GetTileSetInfo().m_Name
	};
}

TileSheet::ParentTileset TileSheet::FindParentTileset(const uint32_t tileGID)
{
	// Work out which tileset it belongs to
	std::string parentTileSetName;

	for (const auto& [firstGid, tsxSource] : m_tmjParser->GetTileSets())
	{
		const int offset = static_cast<int>(tileGID) - static_cast<int>(firstGid) + 1;
		if (offset >= 0)
		{
			for (const auto& [tileSetName, tsxTileSet] : m_tileSets)
			{
				if (tsxSource == tsxTileSet->GetPath())
				{
					return { tileSetName, firstGid };
				}
			}
		}
	}

	return { "", 0xFFFF };
}
