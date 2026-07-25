#ifndef ENTITYREGISTRY_H
#define ENTITYREGISTRY_H
#include "Entity.h"

class EntityRegistry
{
public:
	friend class Renderer;
	template <typename T, typename... Args>
	T& Create(Args&&... args)
		requires (std::is_base_of_v<Entity, T>)
	{
		auto entity = std::make_unique<T>(std::forward<Args>(args)...);
		T& ref = *entity;
		m_entities.emplace(entity->GetID(), std::move(entity));
		return ref;
	}

	template <typename... Args>
	Entity& Create(Args&&... args)
	{
		return Create<Entity>(std::forward<Args>(args)...);
	}

	template<typename T>
	T* Get(const entity_id_t id) const
		requires (std::is_base_of_v<Entity, T>)
	{
		if (!m_entities.contains(id))
		{
			return nullptr;
		}

		return dynamic_cast<T*>(m_entities.at(id).get());
	}

	Entity* Get(const entity_id_t id) const
	{
		return Get<Entity>(id);
	}


	void Destroy(entity_id_t id);

	void UpdateAll(float deltaTime);

	// Returns ~0 if no entity is at the position, otherwise returns the Entity ID
	entity_id_t GetEntityAtPosition(const sf::Vector2f& position);

private:
	std::unordered_map<entity_id_t, std::unique_ptr<Entity>> m_entities;
};

#endif
