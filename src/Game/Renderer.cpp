#include "Renderer.h"

#include <iostream>
#include <SFML/Graphics/RectangleShape.hpp>

#include "../Engine/Asserts.h"
#include "../Engine/EngineEvents.h"
#include "../Engine/Entity.h"
#include "../Engine/EntityRegistry.h"
#include "../Engine/Globals.h"
#include "../Engine/TextureManager.h"
#include "../Engine/Animation/AnimationComponent.h"


Renderer::Renderer() :
	m_zoom(1.f),
	m_cameraView(
		sf::Vector2f(),
		static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize) / m_zoom)
{
	engine_events::OnWindowResized.On([this](const sf::Vector2u size) { OnScreenResized(size); });

	OnScreenResized(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize);
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

void Renderer::SetZoom(const float zoom)
{
	ASSERT(zoom > 0.f);

	m_zoom = zoom;

	// recomputes size + viewport together
	OnScreenResized(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize);
}

float Renderer::GetZoom() const
{
	return m_zoom;
}


void Renderer::BuildBatches(const std::unordered_map<hash_type, LevelRenderData>& visibleLevelRenderData)
{
	m_layerBatchers.clear();

	const auto generateLayerName = [](const hash_type& levelName, const hash_type& layerName) -> hash_type
	{
#if BUILD_DEBUG
		return levelName + "#" + layerName;
#else
		hash_type combinedHash = levelName;

		// boost::hash_combine pattern, stolen from https://gist.github.com/eugene-malashkin/884e225ff57aca1b9cbe
		combinedHash ^= layerName + 0x9e3779b9 + (combinedHash << 6) + (combinedHash >> 2);

		return combinedHash;
#endif
	};

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
		for (const auto& layerMetaData : renderData.m_TileLayerData)
		{
			const hash_type key = generateLayerName(levelName, layerMetaData.m_Name);
			if (auto it = buckets.find(key); it != buckets.end())
			{
				for (auto& [texture, sprites] : it->second)
				{
					SpriteBatcher batcher(texture);
					batcher.BatchSprites(sprites);
					m_layerBatchers.push_back({
						key,
						layerMetaData.m_ZIndex,
						std::move(batcher)
					});
				}
			}
		}
	}

	std::ranges::sort(m_layerBatchers, [](const LayerBatcher& a, const LayerBatcher& b)
	{
		return a.m_ZIndex < b.m_ZIndex;
	});
}

void Renderer::RenderLevel(sf::RenderWindow& window, const EntityRegistry& entities, const int entityZIndex)
{
	m_cameraView.setViewport(GRAPHIC_SETTINGS.GetLetterboxViewport());
	window.setView(m_cameraView);

	for (const auto& layerBatcher : m_layerBatchers)
	{
		window.draw(layerBatcher.m_SpriteBatcher);

		if (layerBatcher.m_ZIndex == entityZIndex)
		{
			RenderEntities(window, entities);
		}
	}

	// Reset the view
	window.setView({
		static_cast<sf::Vector2f>(REFERENCE_SCREEN_SIZE) / 2.f,
		static_cast<sf::Vector2f>(REFERENCE_SCREEN_SIZE)
	});
}

void Renderer::RenderEntity(sf::RenderWindow& window, const Entity* entity) const
{
#if BUILD_DEBUG
	sf::RectangleShape entityBox{ entity->GetSize() };
	entityBox.setFillColor(sf::Color::Magenta);
	entityBox.setPosition(entity->GetPosition());
	window.draw(entityBox);
#endif

	// TODO: We should probably allow static, un-animated, entities!
	const auto* animation = entity->GetComponent<EntityAnimationComponent>();
	if (animation == nullptr)
	{
		return;
	}

	// TODO: What a HORRIBLE way to get the animated sprite!
	const auto& animator = animation->GetAnimator();
	const sf::Texture& texture = *TEXTUREMANAGER.GetTexture(animator.GetDictionarySpritesheetResourceName());

	sf::Sprite sprite(texture);
	const AnimationFrame& currentFrame = animator.GetCurrentFrame();

	sprite.setTextureRect({
		{ static_cast<int>(currentFrame.m_TopLeftX), static_cast<int>(currentFrame.m_TopLeftY) },
		{ static_cast<int>(currentFrame.m_SpriteWidth), static_cast<int>(currentFrame.m_SpriteHeight) }
	});

	const sf::FloatRect bounds = sprite.getLocalBounds();
	const float w = bounds.size.x;
	const float h = bounds.size.y;

	float ox = 0.f, oy = 0.f;

	switch (animator.GetCurrentClip().m_SpriteAnchor)
	{
	case Animation::eSpriteAnchor::TOP_LEFT: ox = 0.f;
		oy = 0.f;
		break;
	case Animation::eSpriteAnchor::TOP_MIDDLE: ox = w / 2.f;
		oy = 0.f;
		break;
	case Animation::eSpriteAnchor::TOP_RIGHT: ox = w;
		oy = 0.f;
		break;
	case Animation::eSpriteAnchor::MIDDLE_LEFT: ox = 0.f;
		oy = h / 2.f;
		break;
	case Animation::eSpriteAnchor::MIDDLE_MIDDLE: ox = w / 2.f;
		oy = h / 2.f;
		break;
	case Animation::eSpriteAnchor::MIDDLE_RIGHT: ox = w;
		oy = h / 2.f;
		break;
	case Animation::eSpriteAnchor::BOTTOM_LEFT: ox = 0.f;
		oy = h;
		break;
	case Animation::eSpriteAnchor::BOTTOM_MIDDLE: ox = w / 2.f;
		oy = h;
		break;
	case Animation::eSpriteAnchor::BOTTOM_RIGHT: ox = w;
		oy = h;
		break;
	}

	sprite.setOrigin({ ox, oy });
	sprite.setPosition(entity->GetPosition() + sf::Vector2f{ 16, 0 });

	/*sprite.setOrigin({ sprite.getLocalBounds().size.x / 2, sprite.getLocalBounds().size.y / 2 });
	sprite.setPosition(entity->GetPosition() + sf::Vector2f{ 16, 0 });*/

	sf::Vector2f spriteScale = entity->GetScale();
	if (currentFrame.m_SpriteFlippedHorizontal)
	{
		spriteScale.x *= -1;
	}

	if (currentFrame.m_SpriteFlippedVertical)
	{
		spriteScale.y *= -1;
	}

	sprite.setScale(spriteScale);

	window.draw(sprite);
}

void Renderer::RenderEntities(sf::RenderWindow& window, const EntityRegistry& entities) const
{
	std::vector<const Entity*> renderables;
	renderables.reserve(entities.m_entities.size());

	for (const auto& entity : entities.m_entities | std::views::values)
	{
		if (entity->IsActive())
		{
			renderables.push_back(entity.get());
		}
	}

	// Sort by Y position for correct draw order
	std::ranges::sort(renderables, [](const Entity* a, const Entity* b) -> bool
	{
		return a->GetPosition().y < b->GetPosition().y;
	});

	for (const auto& entity : renderables)
	{
		RenderEntity(window, entity);
	}
}

void Renderer::OnScreenResized(const sf::Vector2u newSize)
{
	m_cameraView.setCenter(m_cameraView.getCenter());
	m_cameraView.setViewport(GRAPHIC_SETTINGS.GetLetterboxViewport());
	m_cameraView.setSize(static_cast<sf::Vector2f>(REFERENCE_SCREEN_SIZE) / m_zoom);
}
