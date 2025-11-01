#include "TileLogic.h"

#include <iostream>


TileLogic::TileLogic(const TileMapData& data)
{
	m_layers = data.m_Layers;

	for (const auto& layer : data.m_Layers)
	{
		m_zIndexes[layer.m_Name] = layer.m_ZIndex;

		for (size_t i = 0; i < layer.m_LevelData.size(); ++i)
		{
			uint32_t globalID = layer.m_LevelData[i];

			for (const auto& [name, sheet] : data.m_TileSheets)
			{
				const int localID = static_cast<int>(globalID) - static_cast<int>(sheet->GetFirstGID()) + 1;
				if (localID < 0)
				{
					continue;
				}

				if (const TileSheet::TileDefinition* definition = sheet->GetTileDefinition(localID - 1)) {
					TileInstance instance{
						{
							static_cast<float>((i % data.m_NumColumns) * 32),
							static_cast<float>((i / data.m_NumColumns) * 32)
						},
						definition,
						sheet.get(),
						layer.m_Name
					};

					m_tiles.push_back(instance);
				}
			}
		}
	}
}

std::vector<TileRenderData> TileLogic::BuildRenderData() const
{
	std::vector <TileRenderData> renderables;
	renderables.reserve(m_tiles.size());

	for (const auto& tile : m_tiles)
	{
		TileRenderData renderData;
		renderData.m_GlobalID = tile.m_Definition->m_GlobalID;
		renderData.m_LocalID = tile.m_Definition->m_LocalID;
		renderData.m_Position = tile.m_Position;
		renderData.m_Texture = tile.m_ParentSheet->GetTexture();
		renderData.m_LayerName = tile.m_LayerName;
		renderData.m_ZIndex = m_zIndexes.at(tile.m_LayerName);

		int tileWidth = tile.m_ParentSheet->GetTileWidth();
		int tileHeight = tile.m_ParentSheet->GetTileHeight();
		int tilesPerRow = tile.m_ParentSheet->GetNumColumns();

		int localId = tile.m_Definition->m_LocalID; // should already be 0-based
		int x = (localId % tilesPerRow) * tileWidth;
		int y = (localId / tilesPerRow) * tileHeight;

		if (x + tileWidth > renderData.m_Texture->getSize().x || y + tileHeight > renderData.m_Texture->getSize().y)
			std::cout << "Warning: Tile " << localId << " rect out of bounds for " << tile.m_LayerName << "\n";

		renderData.m_TextureRect = { { x, y }, { tileWidth, tileHeight } };

		renderables.push_back(std::move(renderData));
	}

	return renderables;
}
