#include "EntityRegistry.h"

#include <ranges>
#include <SFML/Graphics/Sprite.hpp>

#include "TextureManager.h"
#include "Animation/AnimationComponent.h"

void EntityRegistry::Destroy(const uint32_t id)
{
	if (!m_entities.contains(id))
	{
		return;
	}

	Entity* ent = Get<Entity>(id);

	if (ent == nullptr)
	{
		return;
	}

	if (ent->IsActive())
	{
		ent->OnDeactivate();
	}

	m_entities.at(id).reset(nullptr);

	m_entities.erase(id);
}

void EntityRegistry::UpdateAll(const float deltaTime)
{
	for (auto& entity : m_entities | std::views::values)
	{
		if (entity->IsActive())
		{
			entity->Update(deltaTime);
		}
	}
}

void EntityRegistry::RenderAll(sf::RenderWindow& window) const
{
	std::vector<const Entity*> renderables;
	renderables.reserve(m_entities.size());

	for (const auto& entity : m_entities | std::views::values)
	{
		if (entity->IsActive())
		{
			renderables.push_back(entity.get());
		}
	}

	// Sort by Y position for correct draw order
	std::ranges::sort(renderables, [](const Entity* a, const Entity* b) -> bool{
		return a->GetPosition().y < b->GetPosition().y;
	});

	for (const auto& entity : renderables)
	{
		// TODO: We should probably allow static, un-animated, entities!
		const auto* animation = entity->GetComponent<EntityAnimationComponent>();
		if (animation == nullptr)
		{
			continue;
		}

		// TODO: What a HORRIBLE line of code!
		const sf::Texture& animatedPlayerTexture = *TEXTUREMANAGER.GetTexture(
			animation->GetAnimator().GetDictionarySpritesheetResourceName());

		sf::Sprite sprite(animatedPlayerTexture);
		const AnimationFrame& currentFrame = animation->GetAnimator().GetCurrentFrame();

		sprite.setTextureRect({
			{ static_cast<int>(currentFrame.m_TopLeftX), static_cast<int>(currentFrame.m_TopLeftY) },
			{ static_cast<int>(currentFrame.m_SpriteWidth), static_cast<int>(currentFrame.m_SpriteHeight) }
		});

		sprite.setOrigin({ sprite.getLocalBounds().size.x / 2, sprite.getLocalBounds().size.y / 2 });
		sprite.setPosition(entity->GetPosition() + sf::Vector2f{ 16, 0 });

		sprite.setScale({ 2, 2 });
		window.draw(sprite);
	}
}

uint32_t EntityRegistry::GetEntityAtPosition(const sf::Vector2f& position)
{
	// TODO: This is going to get slow in the future!
	// Some sort of octree maybe?
	for (const auto& entity : m_entities | std::views::values)
	{
		if (!entity->IsActive())
		{
			continue;
		}

		const sf::Vector2f& entityPosition = entity->GetPosition();
		if ((entityPosition - position).lengthSquared() < 32.f * 32.f)
		{
			return entity->GetID();
		}
	}

	return ~0U;
}
