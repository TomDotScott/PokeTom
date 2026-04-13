#include "TileParser.h"

#include <iostream>
#include <sstream>


bool PortalTrigger::AllowsOrientation(eOrientation orientation) const
{
	return static_cast<uint8_t>(orientation) & m_AllowedOrientations;
}

std::shared_ptr<MapData> TileParser::ParseTMJ(const std::filesystem::path& tmjPath)
{
	MapData data;
	const auto tmjParser = TMJ::Create(tmjPath);

	if (!tmjParser)
	{
		std::cout << "TileParser::ParseTMJ: Failed to parse TMJ file " << tmjPath << "\n";
		return nullptr;
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
		data.m_Layers.push_back({ layer.m_Name, layer.m_Data, layer.m_ZIndex, layer.m_IsPlayerLayer });
	}

	const auto& doors = tmjParser->GetPortals();
	data.m_Portals.reserve(doors.size());
	for (const auto& portal : doors)
	{
		// Parse the orientations for the door;
		uint8_t orientation = 0;

		std::stringstream ss(portal.m_Orientation);
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

		PortalTrigger portalData{
			portal.m_Name,
			orientation,
			{
				static_cast<int>(portal.m_Width) / 32,
				static_cast<int>(portal.m_Height) / 32
			},
			{
				static_cast<int>(portal.m_X) / 32,
				static_cast<int>(portal.m_Y) / 32
			}
		};

		data.m_Portals[portal.m_Name] = portalData;
	}

	const auto& spawnPoints = tmjParser->GetSpawnPoints();
	for (const auto& spawnPoint : spawnPoints)
	{
		const SpawnPointData spawnPointData{
			spawnPoint.m_Name,
			StringToOrientation(spawnPoint.m_Orientation),
			{
				static_cast<int>(spawnPoint.m_X) / 32,
				static_cast<int>(spawnPoint.m_Y) / 32
			},
		};

		data.m_SpawnPoints[spawnPoint.m_Name] = spawnPointData;
	}

	data.m_LevelScript = tmjParser->GetLevelScriptFilepath();
	return std::make_shared<MapData>(data);
}
