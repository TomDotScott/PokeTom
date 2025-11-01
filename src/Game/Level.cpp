#include "Level.h"


Level::Level(const std::filesystem::path& levelTMJFilepath) :
	m_mapData(TileParser::ParseTMJ(levelTMJFilepath)),
	m_tileLogic(m_mapData)
{
}

std::vector<TileRenderData> Level::GetRenderData() const
{
	return m_tileLogic.BuildRenderData();
}

std::vector<TileLayerData> Level::GetLayers() const
{
	return m_mapData.m_Layers;
}

uint32_t Level::GetNumColumns() const
{
	return m_mapData.m_NumColumns;
}

uint32_t Level::GetNumRows() const
{
	return m_mapData.m_NumRows;
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
		for (const auto& [sheetName, tileSheet] : m_mapData.m_TileSheets)
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
