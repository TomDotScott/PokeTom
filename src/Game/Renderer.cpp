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

void Renderer::UpdateAnimatedTiles(const float deltaTime)
{
	for (auto& type : m_animatedTileTypes)
	{
		type.m_ElapsedMS += deltaTime;

		const float duration = static_cast<float>(type.m_Frames[type.m_CurrentFrame].m_durationMS) / 1000.f;

		// Only update the UVs of the vertices when the frame actually changes
		if (type.m_ElapsedMS >= duration)
		{
			type.m_ElapsedMS = 0;
			type.m_CurrentFrame = (type.m_CurrentFrame + 1) % type.m_Frames.size();
			PatchUVs(type);
		}
	}
}


void Renderer::BuildBatches(const std::unordered_map<hash_type, LevelRenderData>& visibleLevelRenderData)
{
	m_layerBatchers.clear();
	m_animTypeIndices.clear();
	m_animatedTileTypes.clear();

	const auto CombineHashes = [](const hash_type& levelName, const hash_type& layerName) -> hash_type
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

	struct LayerAccumulator
	{
		int m_zIndex = 0;
		std::unordered_map<hash_type, std::vector<sf::Sprite>> m_byTexture;
	};
	std::unordered_map<hash_type, LayerAccumulator> layerMap;

	for (const auto& [levelHash, renderData] : visibleLevelRenderData)
	{
		std::unordered_map<hash_type, int> layerZIndexes;
		for (const auto& layerData : renderData.m_TileLayerData)
		{
			layerZIndexes[layerData.m_Name] = layerData.m_ZIndex;
		}

		for (const auto& tile : renderData.m_TileRenderData)
		{
			const hash_type key = CombineHashes(levelHash, tile.m_LayerName);
			if (tile.m_AnimatedFrames.has_value())
			{
				const hash_type tileTypeKey = CombineHashes(
					tile.m_SpriteSheetResourceName,
#if BUILD_DEBUG
					std::to_string(tile.m_LocalID)
#else
					tile.m_LocalID
#endif
				);

				auto [it, inserted] = m_animTypeIndices.emplace(tileTypeKey, m_animatedTileTypes.size());
				if (inserted)
				{
					AnimatedTileType type;
					type.m_Texture = TEXTUREMANAGER.GetTexture(tile.m_SpriteSheetResourceName);
					type.m_TileSize = tile.m_TextureRect.size;
					type.m_TextureTilesPerRow = type.m_Texture->getSize().x / type.m_TileSize.x;
					type.m_Frames = tile.m_AnimatedFrames.value();
					type.m_ZIndex = tile.m_ZIndex;
					type.m_Vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
					m_animatedTileTypes.emplace_back(std::move(type));

					PatchUVs(m_animatedTileTypes.back());
				}

				AnimatedTileType& type = m_animatedTileTypes[it->second];
				const size_t startIndex = type.m_Vertices.getVertexCount();

				type.m_Vertices.resize(startIndex + 6);
				const sf::Vector2f p = tile.m_Position;
				const float width = static_cast<float>(type.m_TileSize.x);
				const float height = static_cast<float>(type.m_TileSize.y);

				// Triangle 1 (TL, TR, BL)
				type.m_Vertices[startIndex + 0].position = p;
				type.m_Vertices[startIndex + 1].position = { p.x + width, p.y };
				type.m_Vertices[startIndex + 2].position = { p.x, p.y + height };
				// Triangle 2 (TR, BR, BL)
				type.m_Vertices[startIndex + 3].position = { p.x + width, p.y };
				type.m_Vertices[startIndex + 4].position = { p.x + width, p.y + height };
				type.m_Vertices[startIndex + 5].position = { p.x, p.y + height };


				// We've got the vertex data now so don't need to set up the sprites
				continue;
			}

			auto& accumulator = layerMap[key];
			accumulator.m_zIndex = layerZIndexes[tile.m_LayerName];

			sf::Sprite sprite(*TEXTUREMANAGER.GetTexture(tile.m_SpriteSheetResourceName));
			sprite.setTextureRect(tile.m_TextureRect);
			sprite.setPosition(tile.m_Position);
			accumulator.m_byTexture[tile.m_SpriteSheetResourceName].emplace_back(std::move(sprite));
		}
	}

	for (auto& [key, accumulator] : layerMap)
	{
		for (auto& [textureHash, sprites] : accumulator.m_byTexture)
		{
			SpriteBatcher batcher(textureHash);
			batcher.BatchSprites(sprites);
			m_layerBatchers.emplace_back(key, accumulator.m_zIndex, batcher);
		}
	}

	std::ranges::sort(m_layerBatchers, [](const LayerBatcher& a, const LayerBatcher& b)
	{
		return a.m_ZIndex < b.m_ZIndex;
	});

	std::ranges::sort(m_animatedTileTypes, [](const auto& a, const auto& b)
	{
		return a.m_ZIndex < b.m_ZIndex;
	});
}

void Renderer::RenderLevel(sf::RenderWindow& window, const EntityRegistry& entities, const int entityZIndex)
{
	m_cameraView.setViewport(GRAPHIC_SETTINGS.GetLetterboxViewport());
	window.setView(m_cameraView);

	auto staticTileIterator = m_layerBatchers.begin();
	auto animatedTileIterator = m_animatedTileTypes.begin();

	while (staticTileIterator != m_layerBatchers.end() ||
		animatedTileIterator != m_animatedTileTypes.end())
	{
		const int staticZ = (staticTileIterator != m_layerBatchers.end()) ? staticTileIterator->m_ZIndex : INT_MAX;
		const int animZ = (animatedTileIterator != m_animatedTileTypes.end())
			                  ? animatedTileIterator->m_ZIndex
			                  : INT_MAX;

		if (staticZ <= animZ)
		{
			window.draw(staticTileIterator->m_SpriteBatcher);
			++staticTileIterator;
		}
		else
		{
			sf::RenderStates states;
			states.texture = animatedTileIterator->m_Texture;
			window.draw(animatedTileIterator->m_Vertices, states);
			++animatedTileIterator;
		}

		if (staticZ == entityZIndex)
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

	const sf::Vector2f offsetPosition = entity->GetPosition() + entity->GetOffsetPosition();
	const sf::Vector2f offsetScale = {
		entity->GetScale().x * entity->GetOffsetScale().x,
		entity->GetScale().y * entity->GetOffsetScale().y
	};
	const sf::Angle offsetRotation = entity->GetRotation() + entity->GetOffsetRotation();

	sprite.setPosition(offsetPosition + sf::Vector2f{ 16, 0 });
	sprite.setScale(offsetScale);
	sprite.setRotation(offsetRotation);

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

	// HACKY!!!
	if (sf::Shader* shader = const_cast<Entity*>(entity)->GetCurrentShader())
	{
		shader->setUniform("texture", sf::Shader::CurrentTexture);

		sf::RenderStates states;
		states.shader = shader;
		window.draw(sprite, states);
	}
	else
	{
		window.draw(sprite);
	}
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

void Renderer::PatchUVs(AnimatedTileType& type)
{
	const uint32_t localID = type.m_Frames[type.m_CurrentFrame].m_TileID;
	const int col = localID % type.m_TextureTilesPerRow;
	const int row = localID / type.m_TextureTilesPerRow;
	const float u = static_cast<float>(col * type.m_TileSize.x);
	const float v = static_cast<float>(row * type.m_TileSize.y);
	const float w = static_cast<float>(type.m_TileSize.x);
	const float h = static_cast<float>(type.m_TileSize.y);

	const size_t count = type.m_Vertices.getVertexCount();
	for (size_t i = 0; i < count; i += 6)
	{
		type.m_Vertices[i + 0].texCoords = { u, v }; // TL
		type.m_Vertices[i + 1].texCoords = { u + w, v }; // TR
		type.m_Vertices[i + 2].texCoords = { u, v + h }; // BL

		type.m_Vertices[i + 3].texCoords = { u + w, v }; // TR
		type.m_Vertices[i + 4].texCoords = { u + w, v + h }; // BR
		type.m_Vertices[i + 5].texCoords = { u, v + h }; // BL
	}
}
