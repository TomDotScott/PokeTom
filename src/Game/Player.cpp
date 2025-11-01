#include "Player.h"

#include "../Engine/Globals.h"
#include "../Engine/Input/Keyboard.h"
#include "../Engine/Input/Mouse.h"
#include "../Engine/Timer.h"


Player::Player() :
	GameObject(),
	m_movement(32.f, 5.f),
	m_currentLevel(nullptr)
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
			Move(GridMovementComponent::eDirection::Up);
		}
		else if (m_mapper.IsButtonDown(DOWN))
		{
			Move(GridMovementComponent::eDirection::Down);
		}
		else if (m_mapper.IsButtonDown(LEFT))
		{
			Move(GridMovementComponent::eDirection::Left);
		}
		else if (m_mapper.IsButtonDown(RIGHT))
		{
			Move(GridMovementComponent::eDirection::Right);
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

sf::Vector2i Player::GetGridPosition() const
{
	return m_movement.GetGridPosition();
}

void Player::SetLevel(const Level* level)
{
	m_currentLevel = level;
}

void Player::Move(const GridMovementComponent::eDirection direction)
{
	sf::Vector2i newGridPosition = GetGridPosition();

	switch (direction)
	{
	case GridMovementComponent::eDirection::Up:
		newGridPosition.y -= 1;
		break;
	case GridMovementComponent::eDirection::Down:
		newGridPosition.y += 1;
		break;
	case GridMovementComponent::eDirection::Left:
		newGridPosition.x -= 1;
		break;
	case GridMovementComponent::eDirection::Right:
		newGridPosition.x += 1;
		break;
	}

	if (m_currentLevel->CanMoveTo(newGridPosition.x, newGridPosition.y))
	{
		m_movement.Move(direction);
	}
}
