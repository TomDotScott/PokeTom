#include "Entity.h"

#include "Asserts.h"
#include "GridMovementComponent.h"
#include "Animation/AnimationComponent.h"

Entity::Entity()
{
}

Entity::Entity(const sf::Vector2f& position) :
	GameObject(position),
	IUpdateable()
{
}

void Entity::Update(const float deltaTime)
{
	for (const auto& c : m_components)
	{
		c->Update(deltaTime);
	}

	// Sync position from movement component if one exists
	if (const GridMovementComponent* movement = GetComponent<GridMovementComponent>())
	{
		SetPosition(movement->GetWorldPosition());
	}
}

void Entity::SetPosition(const sf::Vector2f& position)
{
	GameObject::SetPosition(position);

	if (GridMovementComponent* movement = GetComponent<GridMovementComponent>())
	{
		movement->SetWorldPosition(position);
	}
}
