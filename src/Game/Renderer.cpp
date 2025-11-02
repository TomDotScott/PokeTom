#include "Renderer.h"

#include <iostream>

#include "Player.h"
#include "../Engine/Globals.h"

Renderer::Renderer() :
	m_cameraView(sf::Vector2f(), static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize))
{
}

void Renderer::SetCameraCentre(sf::Vector2f position, const uint32_t mapWidth, const uint32_t mapHeight)
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

void Renderer::BuildBatches(const std::vector<TileRenderData>& tiles, const std::vector<TileLayerData>& layers)
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

void Renderer::Render(sf::RenderWindow& window, const Player& player) const
{
	window.setView(m_cameraView);

	for (const auto& layerBatcher : m_layerBatchers)
	{
		window.draw(layerBatcher.m_SpriteBatcher);

		if (layerBatcher.m_ZIndex == player.GetZIndex())
		{
			// TODO: Come up with a proper way to render all the Entities in the level
			// TODO: Make this generic and reusable - everything has walking animations
			sf::Sprite playerSprite(player.GetAnimator().GetTexture());
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
