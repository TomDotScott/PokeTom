#ifndef TILEPARSER_H
#define TILEPARSER_H
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Orientation.h"
#include "TileSheet.h"

struct TileLayerData
{
	std::string m_Name;
	std::vector<uint32_t> m_LevelData;
	int m_ZIndex;
};

struct PortalData
{
	// TODO: This needs to be improved; we shouldn't be loading the levels willy-nilly
	// TODO: Some sort of asset management system that preloads things
	std::filesystem::path m_LevelToLoad;

	// The ID from the corresponding TileMapData::m_SpawnPoints map
	uint32_t m_SpawnPointID;

	// The orientations the player can be in, in order to use the door
	// a bitmask of the values in eOrientation
	uint8_t m_Orientation;
	sf::Vector2i m_Size;
	sf::Vector2i m_GridPosition;
};

struct SpawnPointData
{
	// Same as the key from TileMapData::m_SpawnPoints map, not sure
	// if this NEEDS storing but is nice to have just in case
	uint32_t m_ID;

	// The orientation that the player spawns in when the level is transitioned to
	// from a door
	eOrientation m_Orientation;

	sf::Vector2i m_GridPosition;
};

struct TileMapData
{
	std::vector<TileLayerData> m_Layers;
	std::vector<PortalData> m_Portals;
	std::unordered_map<uint32_t, SpawnPointData> m_SpawnPoints;
	std::unordered_map<std::string, std::shared_ptr<TileSheet>> m_TileSheets;
	uint32_t m_NumColumns;
	uint32_t m_NumRows;
};

class TileParser
{
public:
	static TileMapData ParseTMJ(const std::filesystem::path& tmjPath);
};

#endif
