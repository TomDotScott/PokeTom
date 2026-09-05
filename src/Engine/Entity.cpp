#include "Entity.h"
#include "GridMovementComponent.h"
#include "Animation/AnimationComponent.h"

Entity::Entity() : Entity(sf::Vector2f{}, sf::Vector2f{})
{
}

Entity::Entity(const sf::Vector2f& position, const sf::Vector2f& size) :
	GameObject(position),
	IUpdateable(),
	m_size(size)
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

	for (auto& c : m_components)
	{
		c->OnDestroyed();
		c.reset(nullptr);
	}

	m_components.clear();
}

sf::Vector2f Entity::GetSize()
{
	return m_size;
}

const sf::Vector2f& Entity::GetSize() const
{
	return m_size;
}

{
}

{
}

{
}

void Entity::OnPlayerInteractPressed()
{
	IUpdateable::OnPlayerInteractPressed();

	for (const auto& c : m_components)
	{
		c->OnPlayerInteractPressed();
	}
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
