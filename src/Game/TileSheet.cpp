#include "TileSheet.h"

#include <iostream>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>


TileSheet::TileSheet(const std::shared_ptr<TSX>& tsxParser, const uint32_t firstGID) :
	m_name(tsxParser->GetTileSetInfo().m_Name),
	m_firstGID(firstGID),
	m_numColumns(tsxParser->GetTileSetInfo().m_NumColumns),
	m_tileWidth(tsxParser->GetTileSetInfo().m_TileWidth),
	m_tileHeight(tsxParser->GetTileSetInfo().m_TileHeight)
{
	const auto& imageFilePath = tsxParser->GetImageInfo().m_Source;
	if (!LoadTexture(imageFilePath))
	{
		std::cout << "TileSheet::TileSheet: Error loading texture file from " << imageFilePath << "\n";
	}

	BuildTileDefinitions(tsxParser);
}

const TileSheet::TileDefinition* TileSheet::GetTileDefinition(const uint32_t localID) const
{
	auto it = m_tiles.find(localID);
	if (it == m_tiles.end())
	{
		return nullptr;
	}

	return &it->second;
}

std::shared_ptr<sf::Texture> TileSheet::GetTexture() const
{
	return m_texture;
}

uint32_t TileSheet::GetNumColumns() const
{
	return m_numColumns;
}

uint32_t TileSheet::GetFirstGID() const
{
	return m_firstGID;
}

uint32_t TileSheet::GetTileWidth() const
{
	return m_tileWidth;
}

uint32_t TileSheet::GetTileHeight() const
{
	return m_tileHeight;
}

bool TileSheet::LoadTexture(const std::filesystem::path& filepath)
{
	if (!std::filesystem::exists(filepath))
	{
		std::cout << "TileSheet: Missing texture at " << filepath << "\n";
		return false;
	}

	m_texture = std::make_shared<sf::Texture>();
	if (!m_texture->loadFromFile(filepath))
	{
		std::cout << "TileSheet: Failed to load " << filepath << "\n";
		return false;
	}

	return true;
}

void TileSheet::BuildTileDefinitions(const std::shared_ptr<TSX>& tsxParser)
{
	for (const auto& tsxTile : tsxParser->GetTiles())
	{
		uint32_t flags = TileDefinition::Properties::None;

		for (const auto& tileProperties : tsxTile.m_Properties)
		{
			// TODO: These probably shouldn't be hardcoded!
			if (tileProperties.m_Name == "isBarrier" && tileProperties.m_Value.m_bValue) flags |= TileDefinition::Properties::IsBarrier;
			else if (tileProperties.m_Name == "isBreakable" && tileProperties.m_Value.m_bValue) flags |= TileDefinition::Properties::IsBreakable;
			else if (tileProperties.m_Name == "isGrass" && tileProperties.m_Value.m_bValue) flags |= TileDefinition::Properties::IsGrass;
			else if (tileProperties.m_Name == "isWater" && tileProperties.m_Value.m_bValue) flags |= TileDefinition::Properties::IsWater;
			else if (tileProperties.m_Name == "isDoor" && tileProperties.m_Value.m_bValue) flags |= TileDefinition::Properties::IsDoor;
		}

		m_tiles[tsxTile.m_ID] = {
			tsxTile.m_ID + m_firstGID - 1,
			tsxTile.m_ID,
			flags,
			m_name
		};
	}
}
