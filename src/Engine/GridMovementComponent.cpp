#include "GridMovementComponent.h"

#include <iostream>

#include "Asserts.h"
#include "Animation/AnimationComponent.h"

GridMovementComponent::GridMovementComponent(
	Entity* owner,
	const float tileSize,
	const float walkTilesPerSecond,
	const float sprintTilesPerSecond
) :
	m_owningEntity(owner),
	m_previousDirection(eDirection::None),
	m_currentDirection(eDirection::None),
	m_movingDirection(eDirection::None),
	m_tileSize(tileSize),
	m_speed(walkTilesPerSecond),
	m_sprintSpeed(sprintTilesPerSecond),
	m_isSprinting(false),
	m_progress(0.f),
	m_wasMoving(false),
	m_isMoving(false),
	m_isCycling(false)
{
}

void GridMovementComponent::Move(const eDirection direction)
{
	if (m_isMoving)
	{
		return;
	}

	SetDirection(direction);

	if (CanMove(direction))
	{
		StartMove(GetNextPosition(m_currentDirection));
	}
}

bool GridMovementComponent::CanMove(const eDirection direction) const
{
	return m_canMove && m_canMove(m_owningEntity, direction);
}

void GridMovementComponent::SetDirection(const eOrientation orientation)
{
	m_previousDirection = m_currentDirection;

	switch (orientation)
	{
	case eOrientation::Up:
		m_currentDirection = eDirection::North;
		break;
	case eOrientation::Down:
		m_currentDirection = eDirection::South;
		break;
	case eOrientation::Left:
		m_currentDirection = eDirection::West;
		break;
	case eOrientation::Right:
		m_currentDirection = eDirection::East;
		break;
	}
}

void GridMovementComponent::SetDirection(const eDirection direction)
{
	m_previousDirection = m_currentDirection;
	m_currentDirection = direction;
}

eDirection GridMovementComponent::GetCurrentDirection() const
{
	return m_currentDirection;
}

eDirection GridMovementComponent::GetPreviousDirection() const
{
	return m_previousDirection;
}

void GridMovementComponent::SetCycling(const bool isCycling)
{
	m_isCycling = isCycling;
}

bool GridMovementComponent::IsCycling() const
{
	return m_isCycling;
}

sf::Vector2f GridMovementComponent::GetNextPosition(const eDirection direction) const
{
	sf::Vector2f dir(0, 0);

	switch (direction == eDirection::None ? m_previousDirection : direction)
	{
	case eDirection::North:
		dir += { 0, -1 };
		break;
	case eDirection::South:
		dir += { 0, 1 };
		break;
	case eDirection::West:
		dir += { -1, 0 };
		break;
	case eDirection::East:
		dir += { 1, 0 };
		break;
	default:
		std::cerr << "GridMovementComponent::Move: Unhandled case " << static_cast<int>(m_currentDirection) << "!\n";
		break;
	}

	return m_worldPos + (dir * m_tileSize);
}

void GridMovementComponent::Update(const float deltaTime)
{
	if (m_isMoving != m_wasMoving)
	{
		m_wasMoving = m_isMoving;
	}

	if (!m_isMoving)
	{
		if (m_currentDirection != eDirection::None)
		{
			m_previousDirection = m_currentDirection;
			m_currentDirection = eDirection::None;
		}

		UpdateMovementAnimations();
		return;
	}

	float speed = m_speed;
	if (m_isSprinting)
	{
		speed = m_sprintSpeed;
	}

	if (m_isCycling)
	{
		speed = m_sprintSpeed * 1.1f;
	}

	m_progress += speed * deltaTime;

	if (m_progress >= 1.f)
	{
		m_progress = 1.f;
		m_isMoving = false;
		m_worldPos = m_endPos;
	}

	m_worldPos = m_startPos + (m_endPos - m_startPos) * m_progress;
	UpdateMovementAnimations();
}

const sf::Vector2f& GridMovementComponent::GetWorldPosition() const
{
	return m_worldPos;
}

void GridMovementComponent::SetWorldPosition(const sf::Vector2f& pos)
{
	m_worldPos = pos;
}

bool GridMovementComponent::IsMoving() const
{
	return m_isMoving;
}

bool GridMovementComponent::WasMoving() const
{
	return m_wasMoving;
}

void GridMovementComponent::StopMoving()
{
	m_isMoving = false;
}

void GridMovementComponent::SetSprinting(const bool isSprinting)
{
	m_isSprinting = isSprinting;
}

bool GridMovementComponent::IsSprinting() const
{
	return m_isSprinting;
}

eOrientation GridMovementComponent::GetCurrentOrientation() const
{
	if (IsMoving())
	{
		return GetOrientationFromDirection(m_movingDirection);
	}

	if (m_currentDirection != eDirection::None)
	{
		return GetOrientationFromDirection(m_currentDirection);
	}

	return GetOrientationFromDirection(m_previousDirection);
}

void GridMovementComponent::SetCanMoveCallback(can_move_func callback)
{
	m_canMove = std::move(callback);
}

void GridMovementComponent::StartMove(const sf::Vector2f& dir)
{
	m_isMoving = true;
	m_progress = 0.f;
	m_startPos = m_worldPos;
	m_movingDirection = m_currentDirection;
	m_endPos = dir;
}

void GridMovementComponent::UpdateMovementAnimations() const
{
	EntityAnimationComponent* animationComponent = m_owningEntity->GetComponent<EntityAnimationComponent>();
	if (animationComponent == nullptr)
	{
		return;
	}

	const eOrientation orientation = GetCurrentOrientation();
	if (IsCycling())
	{
		animationComponent->PlayAnimation(EntityAnimationComponent::GetCycleAnimation(orientation), false);
	}
	else if (WasMoving())
	{
		animationComponent->PlayAnimation(IsSprinting()
			? EntityAnimationComponent::GetSprintAnimation(orientation)
			: EntityAnimationComponent::GetWalkAnimation(orientation), false);
	}
	else
	{
		animationComponent->PlayAnimation(EntityAnimationComponent::GetIdleAnimation(orientation), false);
	}
}

