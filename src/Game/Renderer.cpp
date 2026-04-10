#include "Renderer.h"

#include <iostream>

#include "../Engine/Entity.h"
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


void Renderer::BuildBatches(const std::unordered_map<std::string, LevelRenderData>& visibleLevelRenderData)
{
	m_layerBatchers.clear();

	const auto generateLayerName = [](const std::string& levelName, const std::string& layerName) -> std::string{
		return levelName + "#" + layerName;
	};

	std::unordered_map<std::string, std::unordered_map<std::string, std::vector<sf::Sprite>>> buckets;
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

			const auto& layerData = std::find_if(renderData.m_TileLayerData.begin(), renderData.m_TileLayerData.end(), [&](const TileLayerData& a){
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

	std::sort(m_layerBatchers.begin(), m_layerBatchers.end(), [](const LayerBatcher& a, const LayerBatcher& b){
		return a.m_ZIndex < b.m_ZIndex;
	});
}

void Renderer::Render(sf::RenderWindow& window, const std::vector<std::shared_ptr<Entity>>& entities, const int entityZIndex) const
{
	window.setView(m_cameraView);

	for (const auto& layerBatcher : m_layerBatchers)
	{
		window.draw(layerBatcher.m_SpriteBatcher);

		if (layerBatcher.m_ZIndex == entityZIndex)
		{
			for (const auto& entity : entities)
			{
				// TODO: What a HORRIBLE line of code!
				const sf::Texture& animatedPlayerTexture = *TEXTUREMANAGER.GetTexture(entity->GetAnimator().GetDictionarySpritesheetResourceName());

				// TODO: Come up with a proper way to render all the Entities in the level
				// TODO: Make this generic and reusable - everything has walking animations
				sf::Sprite playerSprite(animatedPlayerTexture);
				const AnimationFrame& currentFrame = entity->GetAnimator().GetCurrentFrame();

				playerSprite.setTextureRect({
					{ static_cast<int>(currentFrame.m_TopLeftX), static_cast<int>(currentFrame.m_TopLeftY) },
					{ static_cast<int>(currentFrame.m_SpriteWidth), static_cast<int>(currentFrame.m_SpriteHeight) }
				});

				playerSprite.setOrigin({ playerSprite.getLocalBounds().size.x / 2, playerSprite.getLocalBounds().size.y / 2 });
				playerSprite.setPosition(entity->GetPosition() + sf::Vector2f{ 16, 0 });

				playerSprite.setScale({ 2, 2 });
				window.draw(playerSprite);
			}
		}
	}
}
