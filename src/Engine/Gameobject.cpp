#include "Gameobject.h"

#include "Globals.h"

namespace
{
	uint64_t CURRENT_ID = 0;
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

void GameObject::SetPosition(const sf::Vector2f& position)
{
	m_position = position;
}

uint64_t GameObject::GetID() const
{
	return m_id;
}
