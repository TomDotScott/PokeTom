#ifndef LEVEL_H
#define LEVEL_H
#include "TileLogic.h"
#include "Renderer.h"

class Level
{
public:
	explicit Level(const std::filesystem::path& levelTMJFilepath);

	std::vector<TileRenderData> GetRenderData() const;
	std::vector<TileLayerData> GetLayers() const;

	uint32_t GetNumColumns() const;
	uint32_t GetNumRows() const;

	uint32_t GetPlayerZIndex() const;

	bool CanMoveTo(uint32_t x, uint32_t y) const;
	const DoorData* GetDoorPlayerIsOver(sf::Vector2i playerGridPosition) const;

private:
	TileMapData m_mapData;
	TileLogic m_tileLogic;
};

#endif // LEVEL_H
