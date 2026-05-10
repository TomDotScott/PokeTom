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

	data.m_TileWidth = tmjParser->GetTileWidth();
	data.m_TileHeight = tmjParser->GetTileHeight();

	// Load each layer
	for (const auto& layer : tmjParser->GetLayers())
	{
		data.m_Layers.push_back({
			.m_Name = HASH(layer.m_Name),
			.m_LevelData = layer.m_Data,
			.m_ZIndex = layer.m_ZIndex,
			.m_IsPlayerLayer =
			layer.m_IsPlayerLayer
		});
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
			.m_Name= portal.m_Name,
			.m_AllowedOrientations= orientation,
			.m_Size= {
				static_cast<int>(portal.m_Width / data.m_TileWidth),
				static_cast<int>(portal.m_Height / data.m_TileHeight)
			},
			.m_GridPosition= {
				static_cast<int>(portal.m_X / data.m_TileWidth),
				static_cast<int>(portal.m_Y / data.m_TileHeight)
			}
		};

		data.m_Portals[portal.m_Name] = portalData;
	}

	const auto& spawnPoints = tmjParser->GetSpawnPoints();
	for (const auto& [name, x, y, orientation] : spawnPoints)
	{
		const SpawnPointData spawnPointData{
			.m_Name= name,
			.m_Orientation= StringToOrientation(orientation),
			.m_GridPosition= {
				static_cast<int>(x / data.m_TileWidth),
				static_cast<int>(y / data.m_TileHeight)
			},
		};

		data.m_SpawnPoints[name] = spawnPointData;
	}

	data.m_LevelScript = tmjParser->GetLevelScriptFilepath();
	return std::make_shared<MapData>(data);
}
