#include "TileRenderer.h"

#include <iostream>

void TileRenderer::BuildBatches(const std::vector<TileRenderData>& tiles, const std::vector<TileLayerData>& layers)
{
	m_layerBatchers.clear();

	std::unordered_map<std::string, std::unordered_map<std::shared_ptr<sf::Texture>, std::vector<sf::Sprite>>> buckets;

	for (const auto& tile : tiles)
	{
		sf::Sprite sprite(*tile.m_Texture);
		sprite.setScale({ 0.5, 0.5 });
		sprite.setTextureRect(tile.m_TextureRect);
		sprite.setPosition(tile.m_Position);
		buckets[tile.m_LayerName][tile.m_Texture].push_back(sprite);
	}

	for (auto& bucket : buckets)
	{
		const std::string& layerName = bucket.first;
		const auto& textureGroup = bucket.second;

		const auto& layerData = std::find_if(layers.begin(), layers.end(), [&](const TileLayerData& a)
			{
				return a.m_Name == layerName;
			});

		if (layerData == layers.end())
		{
			std::cout << "TileRenderer::BuildBatcher: Error finding layer data for layerName " << layerName << "\n";
			continue;
		}

		for (auto& [texture, sprites] : textureGroup)
		{
			SpriteBatcher batcher(texture);
			batcher.BatchSprites(sprites);
			m_layerBatchers.push_back({ layerName, layerData->m_ZIndex, std::move(batcher) });
		}
	}

	std::sort(m_layerBatchers.begin(), m_layerBatchers.end(), [](const LayerBatcher& a, const LayerBatcher& b)
		{
			return a.m_ZIndex < b.m_ZIndex;
		});
}

void TileRenderer::Render(sf::RenderWindow& window) const
{
	for (const auto& layerBatcher : m_layerBatchers)
	{
		window.draw(layerBatcher.m_SpriteBatcher);
	}
}
