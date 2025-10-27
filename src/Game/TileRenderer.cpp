#include "TileRenderer.h"

#include <iostream>

void TileRenderer::BuildBatches(const std::vector<TileRenderData>& tiles)
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

	for (auto& [layerName, textureGroup] : buckets)
	{
		for (auto& [texture, sprites] : textureGroup)
		{
			SpriteBatcher batcher(texture);
			batcher.BatchSprites(sprites);
			m_layerBatchers[layerName] = std::move(batcher);
		}
	}
}

void TileRenderer::Render(sf::RenderWindow& window) const
{
	for (const auto& [_, spriteBatcher] : m_layerBatchers)
	{
		window.draw(spriteBatcher);
	}
}
