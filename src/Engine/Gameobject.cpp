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
	printf("%f    %f\n", m_position.x, m_position.y);
}

void GameObject::SetPosition(const float x, const float y)
{
	m_position.x = x;
	m_position.y = y;
}

uint64_t GameObject::GetID() const
{
	return m_id;
}
