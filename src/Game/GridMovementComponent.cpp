#include "GridMovementComponent.h"

#include <iostream>


GridMovementComponent::GridMovementComponent(const float tileSize, const float walkTilesPerSecond, const float sprintTilesPerSecond) :
	m_previousDirection(eDirection::None),
	m_currentDirection(eDirection::None),
	m_tileSize(tileSize),
	m_speed(walkTilesPerSecond),
	m_sprintSpeed(sprintTilesPerSecond),
	m_isSprinting(false),
	m_progress(0.f),
	m_isMoving(false)
{
}

void GridMovementComponent::SetGridPosition(const sf::Vector2i& gridPos)
{
	m_gridPos = gridPos;
	m_worldPos = static_cast<sf::Vector2f>(gridPos) * m_tileSize;
}

void GridMovementComponent::Move(const eDirection direction)
{
	if (m_isMoving)
	{
		return;
	}

	sf::Vector2i newPos = m_gridPos;
	switch (direction)
	{
	case eDirection::North:
		newPos += { 0, -1 };
		break;
	case eDirection::South:
		newPos += { 0, 1 };
		break;
	case eDirection::West:
		newPos += { -1, 0 };
		break;
	case eDirection::East:
		newPos += { 1, 0 };
		break;
	default:
		std::cerr << "GridMovementComponent::Move: Unhandled case " << static_cast<int>(direction) << "!\n";
		break;
	}

	SetDirection(direction);
	StartMove(newPos);
}

void GridMovementComponent::SetDirection(const eDirection direction)
{
	m_previousDirection = m_currentDirection;
	m_currentDirection = direction;
}

GridMovementComponent::eDirection GridMovementComponent::GetCurrentDirection() const
{
	return m_currentDirection;
}

GridMovementComponent::eDirection GridMovementComponent::GetPreviousDirection() const
{
	return m_previousDirection;
}

void GridMovementComponent::Update(const float deltaTime)
{
	if (!m_isMoving)
	{
		if (m_currentDirection != eDirection::None)
		{
			m_previousDirection = m_currentDirection;
			m_currentDirection = eDirection::None;
		}

		return;
	}

	float speed = m_speed;
	if (m_isSprinting)
	{
		speed = m_sprintSpeed;
	}

	m_progress += speed * deltaTime;

	if (m_progress >= 1.f)
	{
		m_progress = 1.f;
		m_isMoving = false;
		m_gridPos = m_targetGridPos;
	}

	m_worldPos = m_startPos + (m_endPos - m_startPos) * m_progress;
}

const sf::Vector2f& GridMovementComponent::GetWorldPosition() const
{
	return m_worldPos;
}

bool GridMovementComponent::IsMoving() const
{
	return m_isMoving;
}

sf::Vector2i GridMovementComponent::GetGridPosition() const
{
	return m_gridPos;
}

void GridMovementComponent::SetSprinting(const bool isSprinting)
{
	m_isSprinting = isSprinting;
}

bool GridMovementComponent::IsSprinting() const
{
	return m_isSprinting;
}

void GridMovementComponent::StartMove(const sf::Vector2i& newGridPos)
{
	m_isMoving = true;
	m_progress = 0.f;
	m_startPos = m_worldPos;
	m_endPos = static_cast<sf::Vector2f>(newGridPos) * m_tileSize;
	m_targetGridPos = newGridPos;
}
