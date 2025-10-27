#include "TileParser.h"

#include <iostream>


TileMapData TileParser::ParseTMJ(const std::filesystem::path& tmjPath)
{
	TileMapData data;
	auto tmjParser = TMJ::Create(tmjPath);

	if (!tmjParser)
	{
		std::cout << "TileParser::ParseTMJ: Failed to parse TMJ file " << tmjPath << "\n";
		return {};
	}

	// Load each tileset
	for (const auto& tileset : tmjParser->GetTileSets())
	{
		auto tsx = TSX::Create(tileset.m_TsxSource);
		if (!tsx)
		{
			std::cout << "TileParser::ParseTMJ: Failed to parse TSX file " << tileset.m_TsxSource << "\n";
			continue;
		}

		auto tileSheet = std::make_shared<TileSheet>(tsx, tileset.m_FirstGid);
		data.m_TileSheets[tsx->GetTileSetInfo().m_Name] = tileSheet;
	}

	data.m_NumColumns = tmjParser->GetNumColumns();

	// Load each layer
	for (const auto& layer : tmjParser->GetLayers())
	{
		// TODO: Create this!
		if (layer.m_Name == "NPC Spawns")
		{
			continue;
		}

		data.m_Layers.push_back({ layer.m_Name, layer.m_Data });
	}

	return data;
}
