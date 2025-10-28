#include "Player.h"

#include "../Engine/Globals.h"
#include "../Engine/Input/Keyboard.h"
#include "../Engine/Input/Mouse.h"
#include "../Engine/Timer.h"


Player::Player() :
	GameObject(),
	m_movement(32.f, 5.f)
{
	m_mapper.Map(UP, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::W));
	m_mapper.Map(DOWN, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::S));

	m_mapper.Map(LEFT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::A));
	m_mapper.Map(RIGHT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::D));
}

void Player::Update(const float deltaTime)
{
	// Update inputs
	m_mapper.Update();

	if (!m_movement.IsMoving())
	{
		if (m_mapper.IsButtonDown(UP))
		{
			m_movement.Move(GridMovementComponent::eDirection::Up);
		}
		else if (m_mapper.IsButtonDown(DOWN))
		{
			m_movement.Move(GridMovementComponent::eDirection::Down);
		}
		else if (m_mapper.IsButtonDown(LEFT))
		{
			m_movement.Move(GridMovementComponent::eDirection::Left);
		}
		else if (m_mapper.IsButtonDown(RIGHT))
		{
			m_movement.Move(GridMovementComponent::eDirection::Right);
		}
	}

	m_position = m_movement.GetWorldPosition();
}

void Player::SetPosition(const float x, const float y)
{
	SetPosition({ x, y });
}

void Player::SetPosition(const sf::Vector2f& position)
{
	GameObject::SetPosition(position);

	m_movement.SetGridPosition({ static_cast<int>(position.x) / 32, static_cast<int>(position.y) / 32 });
}
