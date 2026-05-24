#include "EntityRegistry.h"

#include <ranges>
#if BUILD_DEBUG
#include <SFML/Graphics/RectangleShape.hpp>
#endif
#include <SFML/Graphics/Sprite.hpp>

#include "TextureManager.h"
#include "Animation/AnimationComponent.h"

void EntityRegistry::Destroy(const entity_id_t id)
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
	for (const auto& entity : m_entities | std::views::values)
	{
		if (entity->IsActive())
		{
			entity->Update(deltaTime);
		}
	}
}

entity_id_t EntityRegistry::GetEntityAtPosition(const sf::Vector2f& position)
{
	// TODO: This is going to get slow in the future!
	// Some sort of octree maybe?
	for (const auto& entity : m_entities | std::views::values)
	{
		if (!entity->IsActive())
		{
			continue;
		}

		const sf::Vector2f& entitySize = entity->GetSize();

		const sf::Vector2f& entityTL = entity->GetPosition();
		sf::Vector2f entityBR = entityTL + entitySize;

		if (position.x >= entityTL.x
			&& position.y >= entityTL.y
			&& position.x < entityBR.x
			&& position.y < entityBR.y)
		{
			return entity->GetID();
		}
	}

	return ~0U;
}
