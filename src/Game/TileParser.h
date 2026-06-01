#ifndef TILEPARSER_H
#define TILEPARSER_H
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Engine/Orientation.h"
#include "../Engine/Hash.h"
#include "TileSheet.h"

struct TileLayerData
{
	hash_type m_Name;
	std::vector<uint32_t> m_LevelData;
	int m_ZIndex;
	bool m_IsPlayerLayer;
};

struct PortalTrigger
{
	bool AllowsOrientation(eOrientation orientation) const;

	hash_type m_Name;

	// The orientations the player can be in, in order to use the door
	// a bitmask of the values in eOrientation
	uint8_t m_AllowedOrientations;
	sf::Vector2i m_Size;
	sf::Vector2i m_GridPosition;
};

struct SpawnPointData
{
	hash_type m_Name;

	// The orientation that the player spawns in when the level is transitioned to
	// from a door
	eOrientation m_Orientation;

	sf::Vector2i m_GridPosition;
};

struct MapData
{
	std::vector<TileLayerData> m_Layers;
	std::unordered_map<hash_type, PortalTrigger> m_Portals;
	std::unordered_map<hash_type, SpawnPointData> m_SpawnPoints;
	std::unordered_map<hash_type, std::shared_ptr<TileSheet>> m_TileSheets;
	std::filesystem::path m_LevelScript;

	struct MonsterDistribution final
	{
		enum class eDistributionType
		{
			Grass,
			Water,
			Ambient
		};

		MonsterDistribution(const nlohmann::json& distConfig);

		uint32_t ChooseMonsterFromDistribution(eDistributionType type) const;

		// 0-1, probability for a PocketMonster to spawn every step the player is in Tall Grass
		float m_EncounterProbability;

		// The larger the float, the more likely the PocketMonster is to spawn in a specific area
		std::unordered_map<uint32_t, float> m_GrassWeightings;
		std::unordered_map<uint32_t, float> m_WaterWeightings;
		std::unordered_map<uint32_t, float> m_AmbientWeightings;

	private:
		static bool LoadWeightings(std::unordered_map<uint32_t, float>& weights, const nlohmann::json& weightArrayJSON);
		static uint32_t ChooseFromWeightings(const std::unordered_map<uint32_t, float>& weights);
	};

	// MOST levels have one, but some won't
	std::optional<MonsterDistribution> m_Distribution = std::nullopt;

	uint32_t m_NumColumns;
	uint32_t m_NumRows;
	uint32_t m_TileHeight;
	uint32_t m_TileWidth;
};

class TileParser
{
public:
	static std::shared_ptr<MapData> ParseTMJ(const std::filesystem::path& tmjPath);
};

#endif
