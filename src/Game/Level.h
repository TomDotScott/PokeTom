#ifndef LEVEL_H
#define LEVEL_H
#include "TileLogic.h"
#include "Renderer.h"

class Level
{
public:
	struct AdjacentLevels
	{
		std::string m_North;
		std::string m_South;
		std::string m_East;
		std::string m_West;
	};

	explicit Level(std::string name, const std::shared_ptr<MapData>& mapData, AdjacentLevels adjacentLevels, uint32_t offsetRowsFromOrigin, uint32_t offsetColumnsFromOrigin);

	const std::string& GetName() const;

	std::vector<TileRenderData> GetRenderData() const;
	std::vector<TileLayerData> GetLayers() const;

	uint32_t GetNumColumns() const;
	uint32_t GetNumRows() const;

	bool IsWorldSpacePointOnGrid(const sf::Vector2f& worldSpacePosition) const;
	bool CanMoveTo(const sf::Vector2f& worldSpacePosition) const;
	const PortalTrigger* GetPortalAtPosition(sf::Vector2f worldSpacePosition) const;
	sf::Vector2i GetGridPositionFromWorldPosition(const sf::Vector2f& worldSpacePosition) const;

	const SpawnPointData& GetSpawnPointData(const std::string& name) const;

	const AdjacentLevels& GetAdjacentLevels() const;

	// x = Columns, y = Rows
	const sf::Vector2i& GetOffsetFromOrigin() const;

private:
	std::string m_name;
	std::shared_ptr<MapData> m_mapData;
	AdjacentLevels m_adjacentLevels;

	// x = Columns, y = Rows
	sf::Vector2i m_offsetFromOrigin;

	TileLogic m_tileLogic;
};

#endif // LEVEL_H
