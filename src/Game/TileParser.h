#ifndef TILEPARSER_H
#define TILEPARSER_H
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Engine/Orientation.h"
#include "TileSheet.h"

struct TileLayerData
{
	std::string m_Name;
	std::vector<uint32_t> m_LevelData;
	int m_ZIndex;
	bool m_IsPlayerLayer;
};

struct PortalTrigger
{
	bool AllowsOrientation(eOrientation orientation) const;

	std::string m_Name;

	// The orientations the player can be in, in order to use the door
	// a bitmask of the values in eOrientation
	uint8_t m_AllowedOrientations;
	sf::Vector2i m_Size;
	sf::Vector2i m_GridPosition;
};

struct SpawnPointData
{
	std::string m_Name;

	// The orientation that the player spawns in when the level is transitioned to
	// from a door
	eOrientation m_Orientation;

	sf::Vector2i m_GridPosition;
};

struct MapData
{
	std::vector<TileLayerData> m_Layers;
	std::unordered_map<std::string, PortalTrigger> m_Portals;
	std::unordered_map<std::string, SpawnPointData> m_SpawnPoints;
	std::unordered_map<std::string, std::shared_ptr<TileSheet>> m_TileSheets;
	std::filesystem::path m_LevelScript;
	uint32_t m_NumColumns;
	uint32_t m_NumRows;
};

class TileParser
{
public:
	static std::shared_ptr<MapData> ParseTMJ(const std::filesystem::path& tmjPath);
};

#endif
