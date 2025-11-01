#include "GridMovementComponent.h"

#include <iostream>


GridMovementComponent::GridMovementComponent(const float tileSize, const float tilesPerSecond) :
	m_previousDirection(eDirection::None),
	m_currentDirection(eDirection::None),
	m_tileSize(tileSize),
	m_speed(tilesPerSecond),
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
	case eDirection::Up:
		newPos += { 0, -1 };
		break;
	case eDirection::Down:
		newPos += { 0, 1 };
		break;
	case eDirection::Left:
		newPos += { -1, 0 };
		break;
	case eDirection::Right:
		newPos += { 1, 0 };
		break;
	default:
		std::cerr << "GridMovementComponent::Move: Unhandled case " << static_cast<int>(direction) << "!\n";
		break;
	}

	m_previousDirection = m_currentDirection;
	m_currentDirection = direction;

	StartMove(newPos);
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
		if (m_currentDirection != eDirection::None) {
			m_previousDirection = m_currentDirection;
			m_currentDirection = eDirection::None;
		}

		return;
	}

	m_progress += m_speed * deltaTime;
	if (m_progress >= 1.f)
	{
		m_progress = 1.f;
		m_isMoving = false;
		m_gridPos = m_targetGridPos;
	}

	float t = m_progress;
	t = t * t * (3.f - 2.f * t);

	m_worldPos = m_startPos + (m_endPos - m_startPos) * t;
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

void GridMovementComponent::StartMove(const sf::Vector2i& newGridPos)
{
	m_isMoving = true;
	m_progress = 0.f;
	m_startPos = m_worldPos;
	m_endPos = static_cast<sf::Vector2f>(newGridPos) * m_tileSize;
	m_targetGridPos = newGridPos;
}
