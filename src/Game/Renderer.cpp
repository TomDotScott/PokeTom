#include "Renderer.h"

#include <iostream>

#include "../Engine/Entity.h"
#include "../Engine/EntityRegistry.h"
#include "../Engine/Globals.h"
#include "../Engine/TextureManager.h"

Renderer::Renderer() :
	m_cameraView(sf::Vector2f(), static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize))
{
}

void Renderer::SetCameraCentre(sf::Vector2f position, const sf::FloatRect worldBounds)
{
	const sf::Vector2f viewSize = m_cameraView.getSize();
	const sf::Vector2f halfView = viewSize / 2.f;

	const float minX = worldBounds.position.x + halfView.x;
	const float maxX = worldBounds.position.x + worldBounds.size.x - halfView.x;

	const float minY = worldBounds.position.y + halfView.y;
	const float maxY = worldBounds.position.y + worldBounds.size.y - halfView.y;

	if (worldBounds.size.x < viewSize.x)
	{
		position.x = worldBounds.position.x + worldBounds.size.x * 0.5f;
	}
	else
	{
		position.x = std::clamp(position.x, minX, maxX);
	}

	if (worldBounds.size.y < viewSize.y)
	{
		position.y = worldBounds.position.y + worldBounds.size.y * 0.5f;
	}
	else
	{
		position.y = std::clamp(position.y, minY, maxY);
	}

	m_cameraView.setCenter(position);
}


void Renderer::BuildBatches(const std::unordered_map<hash_type, LevelRenderData>& visibleLevelRenderData)
{
	m_layerBatchers.clear();

#if BUILD_DEBUG
	const auto generateLayerName = [](const std::string& levelName, const std::string& layerName) -> std::string {
		return levelName + "#" + layerName;
		};
#else
	const auto generateLayerName = [](const hash_type& levelName, const hash_type& layerName) -> hash_type {
		return levelName ^ layerName;
		};
#endif

	std::unordered_map<hash_type, std::unordered_map<hash_type, std::vector<sf::Sprite>>> buckets;
	for (const auto& [levelName, renderData] : visibleLevelRenderData)
	{
		for (const auto& tile : renderData.m_TileRenderData)
		{
			sf::Sprite sprite(*TEXTUREMANAGER.GetTexture(tile.m_SpriteSheetResourceName));
			sprite.setTextureRect(tile.m_TextureRect);
			sprite.setPosition(tile.m_Position);
			buckets[generateLayerName(levelName, tile.m_LayerName)][tile.m_SpriteSheetResourceName].push_back(sprite);
		}
	}

	for (const auto& [levelName, renderData] : visibleLevelRenderData)
	{
		for (auto& [layerName, textureGroup] : buckets)
		{
			const std::vector<TileLayerData>& tileLayerData = renderData.m_TileLayerData;

			const auto& layerData = std::ranges::find_if(renderData.m_TileLayerData, [&](const TileLayerData& a) {
				return generateLayerName(levelName, a.m_Name) == layerName;
				});

			if (layerData == tileLayerData.end())
			{
				continue;
			}

			for (auto& [texture, sprites] : textureGroup)
			{
				SpriteBatcher batcher(texture);
				batcher.BatchSprites(sprites);
				m_layerBatchers.push_back({ layerName, layerData->m_ZIndex, std::move(batcher) });
			}
		}
	}

	std::ranges::sort(m_layerBatchers, [](const LayerBatcher& a, const LayerBatcher& b) {
		return a.m_ZIndex < b.m_ZIndex;
		});
}

void Renderer::Render(sf::RenderWindow& window, const EntityRegistry& entities, const int entityZIndex) const
{
	window.setView(m_cameraView);

	for (const auto& layerBatcher : m_layerBatchers)
	{
		window.draw(layerBatcher.m_SpriteBatcher);

		if (layerBatcher.m_ZIndex == entityZIndex)
		{
			entities.RenderAll(window);
		}
	}
}
