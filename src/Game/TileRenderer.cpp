#include "TileRenderer.h"

#include <iostream>

#include "../Engine/Globals.h"

TileRenderer::TileRenderer() :
	m_cameraView(sf::Vector2f(), static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize))
{
}

void TileRenderer::SetCameraCentre(sf::Vector2f position, const uint32_t mapWidth, const uint32_t mapHeight)
{
	const sf::Vector2f halfSize = m_cameraView.getSize() / 2.f;
	const float left = halfSize.x;
	const float top = halfSize.y;
	const float right = static_cast<float>(mapWidth) * 32.f - halfSize.x;
	const float bottom = static_cast<float>(mapHeight) * 32.f - halfSize.y;

	position.x = std::clamp(position.x, left, right);
	position.y = std::clamp(position.y, top, bottom);
	m_cameraView.setCenter(position);
}

void TileRenderer::BuildBatches(const std::vector<TileRenderData>& tiles, const std::vector<TileLayerData>& layers)
{
	m_layerBatchers.clear();

	std::unordered_map<std::string, std::unordered_map<std::shared_ptr<sf::Texture>, std::vector<sf::Sprite>>> buckets;

	for (const auto& tile : tiles)
	{
		sf::Sprite sprite(*tile.m_Texture);
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
	window.setView(m_cameraView);

	for (const auto& layerBatcher : m_layerBatchers)
	{
		window.draw(layerBatcher.m_SpriteBatcher);
	}
}
