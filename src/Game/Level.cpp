#include "Level.h"

#include <complex.h>


Level::Level(std::string name, const std::shared_ptr<MapData>& mapData, AdjacentLevels adjacentLevels) :
	m_name(std::move(name)),
	m_mapData(mapData),
	m_adjacentLevels(std::move(adjacentLevels)),
	m_worldTileOrigin(0, 0),
	m_tileLogic(m_mapData)
{
}

const std::string& Level::GetName() const
{
	return m_name;
}

std::vector<TileRenderData> Level::GetRenderData() const
{
	return m_tileLogic.BuildRenderData(m_worldTileOrigin);
}

std::vector<TileLayerData> Level::GetLayers() const
{
	return m_mapData->m_Layers;
}

uint32_t Level::GetNumColumns() const
{
	return m_mapData->m_NumColumns;
}

uint32_t Level::GetNumRows() const
{
	return m_mapData->m_NumRows;
}

bool Level::IsWorldSpacePointOnGrid(const sf::Vector2f& worldSpacePosition) const
{
	const sf::Vector2i gridPos = GetGridPositionFromWorldPosition(worldSpacePosition);

	return gridPos.x >= 0
		&& gridPos.x < static_cast<int>(GetNumColumns())
		&& gridPos.y >= 0
		&& gridPos.y < static_cast<int>(GetNumRows());
}

bool Level::CanMoveTo(const sf::Vector2f& worldSpacePosition) const
{
	if (!IsWorldSpacePointOnGrid(worldSpacePosition))
	{
		return false;
	}

	const sf::Vector2i gridPos = GetGridPositionFromWorldPosition(worldSpacePosition);
	const uint32_t index = gridPos.y * GetNumColumns() + gridPos.x;

	for (const TileLayerData& layer : GetLayers())
	{
		// Skip any layer that doesn't have this index
		if (index >= layer.m_LevelData.size())
		{
			continue;
		}

		const uint32_t globalTileID = layer.m_LevelData[index];
		if (globalTileID <= 0) {
			continue;
		}

		// Find which TileSheet this ID belongs to
		for (const auto& [sheetName, tileSheet] : m_mapData->m_TileSheets)
		{
			if (globalTileID < tileSheet->GetFirstGID())
			{
				continue;
			}

			const uint32_t localID = globalTileID - tileSheet->GetFirstGID();

			const TileSheet::TileDefinition* tileDef = tileSheet->GetTileDefinition(localID);
			if (!tileDef)
			{
				continue;
			}

			// Check collision flag
			if (tileDef->m_Flags & TileSheet::TileDefinition::IsBarrier)
			{
				return false;
			}
		}
	}

	return true;
}

const PortalTrigger* Level::GetPortalAtPosition(const sf::Vector2f worldSpacePosition) const
{
	const sf::Vector2i gridPosition = GetGridPositionFromWorldPosition(worldSpacePosition);
	for (const auto& [_, portalData] : m_mapData->m_Portals)
	{
		const bool xEqual = gridPosition.x == portalData.m_GridPosition.x || gridPosition.x == portalData.m_GridPosition.x + (portalData.m_Size.x - 1);

		const bool yEqual = gridPosition.y == portalData.m_GridPosition.y || gridPosition.y == portalData.m_GridPosition.y + (portalData.m_Size.y - 1);

		if (xEqual && yEqual)
		{
			return &portalData;
		}
	}

	return nullptr;
}

const SpawnPointData& Level::GetSpawnPointData(const std::string& name) const
{
#if BUILD_DEBUG
	// If the map isn't found, assume we are debug-teleporting to a place
	if (m_mapData->m_SpawnPoints.find(name) == m_mapData->m_SpawnPoints.end())
	{
		return m_mapData->m_SpawnPoints.begin()->second;
	}
#endif
	return m_mapData->m_SpawnPoints.at(name);
}

const Level::AdjacentLevels& Level::GetAdjacentLevels() const
{
	return m_adjacentLevels;
}

void Level::SetWorldOrigin(const sf::Vector2i& tileOrigin)
{
	m_worldTileOrigin = tileOrigin;
}

sf::FloatRect Level::GetBounds() const
{
	return {
		static_cast<sf::Vector2f>(m_worldTileOrigin),
		{ static_cast<float>(GetNumColumns()) * 32.f, static_cast<float>(GetNumRows()) * 32.f}
	};
}

const sf::Vector2i& Level::GetWorldOrigin() const
{
	return m_worldTileOrigin;
}

sf::Vector2i Level::GetGridPositionFromWorldPosition(const sf::Vector2f& worldSpacePosition) const
{
	const sf::Vector2i localPosition = static_cast<sf::Vector2i>(worldSpacePosition) + m_worldTileOrigin;

	// Floor/truncate to get integer tile coordinates
	const int gx = static_cast<int>(std::floor(localPosition.x / 32.f));
	const int gy = static_cast<int>(std::floor(localPosition.y / 32.f));
	return { gx, gy };
}

