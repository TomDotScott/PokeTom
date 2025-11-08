#include "TileParser.h"

#include <iostream>


TileMapData TileParser::ParseTMJ(const std::filesystem::path& tmjPath)
{
	TileMapData data;
	const auto tmjParser = TMJ::Create(tmjPath);

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

		const auto tileSheet = std::make_shared<TileSheet>(tsx, tileset.m_FirstGid);
		data.m_TileSheets[tsx->GetTileSetInfo().m_Name] = tileSheet;
	}

	data.m_NumColumns = tmjParser->GetNumColumns();
	data.m_NumRows = tmjParser->GetNumRows();

	// Load each layer
	for (const auto& layer : tmjParser->GetLayers())
	{
		// TODO: Create this!
		if (layer.m_Name == "NPC Spawns")
		{
			continue;
		}

		data.m_Layers.push_back({ layer.m_Name, layer.m_Data, layer.m_ZIndex });
	}

	const auto& doors = tmjParser->GetDoors();
	data.m_Doors.reserve(doors.size());
	for (const auto& door : doors)
	{
		DoorData doorData{
			door.m_LevelToLoad,
			{
				static_cast<int>(door.m_Width) / 32,
				static_cast<int>(door.m_Height) / 32
			},
			{
				static_cast<int>(door.m_X) / 32,
				static_cast<int>(door.m_Y) / 32
			}
		};

		data.m_Doors.emplace_back(doorData);
	}

	return data;
}
