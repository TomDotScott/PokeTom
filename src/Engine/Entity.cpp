#include "Entity.h"

#include "Asserts.h"
#include "GridMovementComponent.h"
#include "Animation/AnimationComponent.h"

Entity::Entity() : Entity(sf::Vector2f{})
{
}

Entity::Entity(const sf::Vector2f& position) :
	GameObject(position),
	IUpdateable()
{
}

Entity::~Entity()
{
	Entity::OnEntityDestroyed();
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

void Entity::OnActivate()
{
	GameObject::OnActivate();

	OnEntityActivate();
}

void Entity::OnDeactivate()
{
	GameObject::OnDeactivate();

	OnEntityDeactivate();
}

void Entity::OnEntityDestroyed()
{
	IUpdateable::OnDestroyed();

	for (const auto& c : m_components)
	{
		c->OnDestroyed();
	}

	m_components.clear();
}

void Entity::OnEntityActivate()
{
	IUpdateable::OnActivate();

	for (const auto& c : m_components)
	{
		c->OnActivate();
	}
}

void Entity::OnEntityDeactivate()
{
	IUpdateable::OnDeactivate();

	for (const auto& c : m_components)
	{
		c->OnDeactivate();
	}
}
