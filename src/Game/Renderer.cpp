#include "Renderer.h"

#include <iostream>

#include "Player.h"
#include "../Engine/Globals.h"
#include "../Engine/TextureManager.h"

Renderer::Renderer() :
	m_cameraView(sf::Vector2f(), static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize))
{
}

void Renderer::SetCameraCentre(sf::Vector2f position, sf::FloatRect worldBounds)
{
	const sf::Vector2f viewSize = m_cameraView.getSize();
	const sf::Vector2f halfView = viewSize / 2.f;

	const float minX = worldBounds.position.x + halfView.x;
	const float maxX = worldBounds.position.x + worldBounds.size.x - halfView.x;

	const float minY = worldBounds.position.y + halfView.y;
	const float maxY = worldBounds.position.y + worldBounds.size.y - halfView.y;

	if (worldBounds.size.x < viewSize.x)
	{
		position.x = worldBounds.size.x / 2.f;
	}
	else
	{
		position.x = std::clamp(position.x, minX, maxX);
	}

	if (worldBounds.size.y < viewSize.y)
	{
		position.y = worldBounds.size.y / 2.f;
	}
	else
	{
		position.y = std::clamp(position.y, minY, maxY);
	}

	m_cameraView.setCenter(position);
}


void Renderer::BuildBatches(const std::vector<TileRenderData>& tiles, const std::vector<TileLayerData>& layers)
{
	m_layerBatchers.clear();

	std::unordered_map<std::string, std::unordered_map<std::string, std::vector<sf::Sprite>>> buckets;

	for (const auto& tile : tiles)
	{
		sf::Sprite sprite(*TEXTUREMANAGER.GetTexture(tile.m_SpriteSheetResourceName));
		sprite.setTextureRect(tile.m_TextureRect);
		sprite.setPosition(tile.m_Position);
		buckets[tile.m_LayerName][tile.m_SpriteSheetResourceName].push_back(sprite);
	}

	for (auto& [layerName, textureGroup] : buckets)
	{
		const auto& layerData = std::find_if(layers.begin(), layers.end(), [&](const TileLayerData& a)
			{
				return a.m_Name == layerName;
			});

		if (layerData == layers.end())
		{
			std::cout << "Renderer::BuildBatcher: Error finding layer data for layerName " << layerName << "\n";
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

void Renderer::Render(sf::RenderWindow& window, const Player& player, const int entityZIndex) const
{
	window.setView(m_cameraView);

	for (const auto& layerBatcher : m_layerBatchers)
	{
		window.draw(layerBatcher.m_SpriteBatcher);

		if (layerBatcher.m_ZIndex == entityZIndex)
		{
			// TODO: What a HORRIBLE line of code!
			const sf::Texture& animatedPlayerTexture = *TEXTUREMANAGER.GetTexture(player.GetAnimator().GetDictionarySpritesheetResourceName());

			// TODO: Come up with a proper way to render all the Entities in the level
			// TODO: Make this generic and reusable - everything has walking animations
			sf::Sprite playerSprite(animatedPlayerTexture);
			const AnimationFrame& currentFrame = player.GetAnimator().GetCurrentFrame();

			playerSprite.setTextureRect({
				{ static_cast<int>(currentFrame.m_TopLeftX), static_cast<int>(currentFrame.m_TopLeftY) },
				{ static_cast<int>(currentFrame.m_SpriteWidth), static_cast<int>(currentFrame.m_SpriteHeight) }
				});

			playerSprite.setOrigin({ playerSprite.getLocalBounds().size.x / 2, playerSprite.getLocalBounds().size.y / 2 });
			playerSprite.setPosition(player.GetPosition() + sf::Vector2f{ 16, 0 });

			playerSprite.setScale({ 2, 2 });
			window.draw(playerSprite);
		}
	}
}
