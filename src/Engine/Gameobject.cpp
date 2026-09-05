#include "Gameobject.h"

namespace
{
	uint32_t CURRENT_ID = 0;
}

GameObject::GameObject(const sf::Vector2f position) :
	m_isActive(false),
	m_id(++CURRENT_ID)
{
	SetPosition(position);
}

void GameObject::OnActivate()
{
	m_isActive = true;
}

void GameObject::OnDeactivate()
{
	m_isActive = false;
}

sf::Vector2f GameObject::GetPosition() const
{
	return m_transformable.getPosition();
}

void GameObject::SetPosition(const sf::Vector2f& position)
{
	m_transformable.setPosition(position);
}

void GameObject::SetPosition(const float x, const float y)
{
	SetPosition({ x, y });
}

sf::Angle GameObject::GetRotation() const
{
	return m_transformable.getRotation();
}

void GameObject::SetRotation(const sf::Angle& angle)
{
	return m_transformable.setRotation(angle);
}

sf::Vector2f GameObject::GetScale() const
{
	return m_transformable.getScale();
}

void GameObject::SetScale(const sf::Vector2f& scale)
{
	return m_transformable.setScale(scale);
}

uint32_t GameObject::GetID() const
{
	return m_id;
}
