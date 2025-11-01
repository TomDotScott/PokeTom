#ifndef LEVEL_H
#define LEVEL_H
#include "TileLogic.h"

class Level
{
public:
	explicit Level(const std::filesystem::path& levelTMJFilepath);

	std::vector<TileRenderData> GetRenderData() const;
	std::vector<TileLayerData> GetLayers() const;

	uint32_t GetNumColumns() const;
	uint32_t GetNumRows() const;

	bool CanMoveTo(uint32_t x, uint32_t y) const;

private:
	TileMapData m_mapData;
	TileLogic m_tileLogic;
};

#endif // LEVEL_H
