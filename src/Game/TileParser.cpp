#include "TileParser.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "../Engine/Asserts.h"


bool PortalTrigger::AllowsOrientation(eOrientation orientation) const
{
	return static_cast<uint8_t>(orientation) & m_AllowedOrientations;
}

MapData::MonsterDistribution::MonsterDistribution(const nlohmann::json& distConfig)
{
	ASSERT(distConfig.contains("EncounterProbability"));
	ASSERT(distConfig.contains("Grass"));
	ASSERT(distConfig.contains("Water"));
	ASSERT(distConfig.contains("Ambient"));

	m_EncounterProbability = distConfig["EncounterProbability"];
	ASSERT(LoadWeightings(m_GrassWeightings, distConfig["Grass"]));
	ASSERT(LoadWeightings(m_WaterWeightings, distConfig["Water"]));
	ASSERT(LoadWeightings(m_AmbientWeightings, distConfig["Ambient"]));
}

uint32_t MapData::MonsterDistribution::ChooseMonsterFromDistribution(const eDistributionType type) const
{
	switch (type)
	{
	case eDistributionType::Grass:
		return ChooseFromWeightings(m_GrassWeightings);
	case eDistributionType::Water:
		return ChooseFromWeightings(m_WaterWeightings);
	case eDistributionType::Ambient:
		return ChooseFromWeightings(m_AmbientWeightings);
	}

	return ChooseFromWeightings(m_GrassWeightings);
}

bool MapData::MonsterDistribution::LoadWeightings(std::unordered_map<uint32_t, float>& weights,
                                                  const nlohmann::json& weightArrayJSON)
{
	for (const auto& weight : weightArrayJSON)
	{
		if (!weight.contains("id") || !weight.contains("weight"))
		{
			return false;
		}

		const uint64_t monsterID = weight["id"];
		const float monsterWeighting = weight["weight"];
		weights[monsterID] = monsterWeighting;
	}

	return true;
}

uint32_t MapData::MonsterDistribution::ChooseFromWeightings(const std::unordered_map<uint32_t, float>& weights)
{
	float sumWeights = 0;
	for (const float f : weights | std::views::values)
	{
		sumWeights += f;
	}

	std::vector<std::pair<uint32_t, float>> sorted(weights.begin(), weights.end());
	std::ranges::sort(sorted);

	RandomRangeGenerator rng(0.f, sumWeights);
	float randomVal = rng.Next();
	float runningTotal = 0.f;

	uint32_t lastID = sorted.back().first;
	for (const auto& [id, weight] : sorted)
	{
		lastID = id;
		runningTotal += weight;
		if (runningTotal > randomVal)
		{
			return id;
		}
	}

	return lastID;
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
		data.m_TileSheets[HASH(tsx->GetTileSetInfo().m_Name)] = tileSheet;
	}

	data.m_NumColumns = tmjParser->GetNumColumns();
	data.m_NumRows = tmjParser->GetNumRows();

	data.m_TileWidth = tmjParser->GetTileWidth();
	data.m_TileHeight = tmjParser->GetTileHeight();

	// Load the Monster Distribution for the level
	for (const auto& [propertyName, propertyValue] : tmjParser->GetCustomProperties())
	{
		if (propertyName == "distribution")
		{
			const bool fileExists = std::filesystem::exists(propertyValue);
			ASSERT(fileExists);
			if (!fileExists)
			{
				return nullptr;
			}

			std::ifstream f;
			f.open(propertyValue);

			nlohmann::json distConfig = nlohmann::json::parse(f);

			data.m_Distribution = MapData::MonsterDistribution(distConfig);
		}
	}


	// Load each layer
	for (const auto& layer : tmjParser->GetLayers())
	{
		data.m_Layers.push_back({
			.m_Name = HASH(layer.m_Name),
			.m_LevelData = layer.m_Data,
			.m_ZIndex = layer.m_ZIndex,
			.m_IsPlayerLayer = layer.m_IsPlayerLayer
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

		auto nameHash = HASH(portal.m_Name);
		PortalTrigger portalData{
			.m_Name = nameHash,
			.m_AllowedOrientations = orientation,
			.m_Size = {
				static_cast<int>(portal.m_Width / data.m_TileWidth),
				static_cast<int>(portal.m_Height / data.m_TileHeight)
			},
			.m_GridPosition = {
				static_cast<int>(portal.m_X / data.m_TileWidth),
				static_cast<int>(portal.m_Y / data.m_TileHeight)
			}
		};

		data.m_Portals[nameHash] = portalData;
	}

	const auto& spawnPoints = tmjParser->GetSpawnPoints();
	for (const auto& [name, x, y, orientation] : spawnPoints)
	{
		auto hashName = HASH(name);
		const SpawnPointData spawnPointData{
			.m_Name = hashName,
			.m_Orientation = StringToOrientation(orientation),
			.m_GridPosition = {
				static_cast<int>(x / data.m_TileWidth),
				static_cast<int>(y / data.m_TileHeight)
			},
		};

		data.m_SpawnPoints[hashName] = spawnPointData;
	}

	data.m_LevelScript = tmjParser->GetLevelScriptFilepath();
	return std::make_shared<MapData>(data);
}
