#include "Level.h"


Level::Level(const std::shared_ptr<MapData>& mapData, AdjacentLevels adjacentLevels) :
	m_mapData(mapData),
	m_adjacentLevels(std::move(adjacentLevels)),
	m_tileLogic(m_mapData)
{
}

std::vector<TileRenderData> Level::GetRenderData() const
{
	return m_tileLogic.BuildRenderData();
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

uint32_t Level::GetPlayerZIndex() const
{
	// TODO: Integrate this with the level somehow
	return 2;
}

bool Level::CanMoveTo(const uint32_t x, const uint32_t y) const
{
	if (x >= GetNumColumns() || y >= GetNumRows())
	{
		return false;
	}

	const uint32_t index = y * GetNumColumns() + x;

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

const PortalTrigger* Level::GetPortalAtPlayerPosition(const sf::Vector2i playerGridPosition) const
{
	for (const auto& [_, portalData] : m_mapData->m_Portals)
	{
		const bool xEqual = playerGridPosition.x == portalData.m_GridPosition.x || playerGridPosition.x == portalData.m_GridPosition.x + (portalData.m_Size.x - 1);

		const bool yEqual = playerGridPosition.y == portalData.m_GridPosition.y || playerGridPosition.y == portalData.m_GridPosition.y + (portalData.m_Size.y - 1);

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

