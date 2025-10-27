#ifndef TILEPARSER_H
#define TILEPARSER_H
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "TileSheet.h"

struct TileLayerData
{
	std::string m_Name;
	std::vector<uint32_t> m_LevelData;
	int m_ZIndex;
};

struct TileMapData
{
	std::vector<TileLayerData> m_Layers;
	std::unordered_map<std::string, std::shared_ptr<TileSheet>> m_TileSheets;
	uint32_t m_NumColumns;
};

class TileParser
{
public:
	static TileMapData ParseTMJ(const std::filesystem::path& tmjPath);
};

#endif
