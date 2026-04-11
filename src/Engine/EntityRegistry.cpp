#include "EntityRegistry.h"

#include <ranges>
#include <SFML/Graphics/Sprite.hpp>

#include "TextureManager.h"
#include "Animation/AnimationComponent.h"

void EntityRegistry::Destroy(const uint64_t id)
{
	Entity* ent = Get<Entity>(id);

	if (ent == nullptr)
	{
		return;
	}

	if (ent->IsActive())
	{
		ent->OnDeactivate();
	}

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
	for (const auto& entity : m_entities | std::views::values)
	{
		const EntityAnimationComponent* animation = entity->GetComponent<EntityAnimationComponent>();
		if (animation == nullptr)
		{
			continue;
		}

		// TODO: What a HORRIBLE line of code!
		const sf::Texture& animatedPlayerTexture = *TEXTUREMANAGER.GetTexture(
			animation->GetAnimator().GetDictionarySpritesheetResourceName());

		// TODO: Come up with a proper way to render all the Entities in the level
		// TODO: Make this generic and reusable - everything has walking animations
		sf::Sprite playerSprite(animatedPlayerTexture);
		const AnimationFrame& currentFrame = animation->GetAnimator().GetCurrentFrame();

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
