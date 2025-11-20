#include "TileLogic.h"

#include <iostream>

#include "../Engine/TextureManager.h"


TileLogic::TileLogic(const std::shared_ptr<MapData>& mapData)
{
	m_layers = mapData->m_Layers;

	for (const auto& layer : mapData->m_Layers)
	{
		m_zIndexes[layer.m_Name] = layer.m_ZIndex;

		for (size_t i = 0; i < layer.m_LevelData.size(); ++i)
		{
			uint32_t globalID = layer.m_LevelData[i];

			for (const auto& [name, sheet] : mapData->m_TileSheets)
			{
				const int localID = static_cast<int>(globalID) - static_cast<int>(sheet->GetFirstGID()) + 1;
				if (localID < 0)
				{
					continue;
				}

				if (const TileSheet::TileDefinition* definition = sheet->GetTileDefinition(localID - 1)) {
					TileInstance instance{
						{
							static_cast<float>((i % mapData->m_NumColumns) * 32),
							static_cast<float>((i / mapData->m_NumColumns) * 32)
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
		renderData.m_SpriteSheetResourceName = tile.m_ParentSheet->GetSpriteSheetResourceName();
		renderData.m_LayerName = tile.m_LayerName;
		renderData.m_ZIndex = m_zIndexes.at(tile.m_LayerName);

		const int tileWidth = tile.m_ParentSheet->GetTileWidth();
		const int tileHeight = tile.m_ParentSheet->GetTileHeight();
		const int tilesPerRow = tile.m_ParentSheet->GetNumColumns();

		const int localId = tile.m_Definition->m_LocalID; // should already be 0-based
		const int x = (localId % tilesPerRow) * tileWidth;
		const int y = (localId / tilesPerRow) * tileHeight;

		const sf::Texture* spriteSheetTexture = TEXTUREMANAGER.GetTexture(renderData.m_SpriteSheetResourceName);
		if (spriteSheetTexture == nullptr)
		{
			continue;
		}

		if (x + tileWidth > spriteSheetTexture->getSize().x || y + tileHeight > spriteSheetTexture->getSize().y)
		{
			std::cout << "Warning: Tile " << localId << " rect out of bounds for " << tile.m_LayerName << "\n";
			continue;
		}

		renderData.m_TextureRect = { { x, y }, { tileWidth, tileHeight } };

		renderables.push_back(std::move(renderData));
	}

	return renderables;
}
