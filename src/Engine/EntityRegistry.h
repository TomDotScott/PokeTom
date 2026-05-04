#ifndef ENTITYREGISTRY_H
#define ENTITYREGISTRY_H
#include "Entity.h"

class EntityRegistry
{
public:
	template <typename T, typename... Args>
	T& Create(Args&&... args)
		requires (std::is_base_of_v<Entity, T>)
	{
		auto entity = std::make_unique<T>(std::forward<Args>(args)...);
		T& ref = *entity;
		m_entities.emplace(entity->GetID(), std::move(entity));
		return ref;
	}

	template<typename T>
	T* Get(const uint32_t id) const
		requires (std::is_base_of_v<Entity, T>)
	{
		if (!m_entities.contains(id))
		{
			return nullptr;
		}

		return dynamic_cast<T*>(m_entities.at(id).get());
	}


	void Destroy(uint32_t id);

	void UpdateAll(float deltaTime);
	void RenderAll(sf::RenderWindow& window) const;

	// Returns ~0 if no entity is at the position, otherwise returns the Entity ID
	uint32_t GetEntityAtPosition(const sf::Vector2f& position);

	void OnInteractPressed(uint32_t id);

private:
	std::unordered_map<uint32_t, std::unique_ptr<Entity>> m_entities;
};

#endif
