#include "TileParser.h"

#include <iostream>
#include <sstream>


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
		// Parse the orientations for the door;
		uint8_t orientation = 0;

		std::stringstream ss(door.m_Orientation);
		std::vector<std::string> splitString;

		while (ss.good())
		{
			std::string substr;
			getline(ss, substr, ',');
			splitString.push_back(substr);
		}

		for (const auto& orientationString : splitString)
		{
			eOrientation o = StringToOrientation(orientationString);
			orientation |= static_cast<uint32_t>(o);
		}

		DoorData doorData{
			door.m_LevelToLoad,
			door.m_SpawnPointID,
			orientation,
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

	const auto& spawnPoints = tmjParser->GetSpawnPoints();
	data.m_SpawnPoints.reserve(spawnPoints.size());
	for (const auto& spawnPoint : spawnPoints)
	{
		const SpawnPointData spawnPointData{
			spawnPoint.m_ID,
			StringToOrientation(spawnPoint.m_Orientation),
			{
				static_cast<int>(spawnPoint.m_X) / 32,
				static_cast<int>(spawnPoint.m_Y) / 32
			},
		};

		data.m_SpawnPoints[spawnPoint.m_ID] = spawnPointData;
	}

	return data;
}
