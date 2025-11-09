#ifndef GRIDMOVEMENTCOMPONENT_H
#define GRIDMOVEMENTCOMPONENT_H
#include <SFML/System/Vector2.hpp>

#include "Orientation.h"
#include "../Engine/Updateable.h"

class GridMovementComponent : Updateable
{
public:
	enum class eDirection
	{
		None = -1,
		North,
		South,
		West,
		East
	};

	GridMovementComponent(float tileSize, float walkTilesPerSecond, float sprintTilesPerSecond);

	void SetGridPosition(const sf::Vector2i& gridPos);
	void Move(eDirection direction);

	void SetDirection(eDirection direction);

	eDirection GetCurrentDirection() const;
	eDirection GetPreviousDirection() const;

	void Update(float deltaTime) override;
	const sf::Vector2f& GetWorldPosition() const;

	bool IsMoving() const;
	sf::Vector2i GetGridPosition() const;

	void SetSprinting(bool isSprinting);
	bool IsSprinting() const;

private:
	sf::Vector2i m_gridPos;
	sf::Vector2i m_targetGridPos;
	sf::Vector2f m_startPos;
	sf::Vector2f m_endPos;
	sf::Vector2f m_worldPos;

	eDirection m_previousDirection;
	eDirection m_currentDirection;

	float m_tileSize;
	float m_speed;
	float m_sprintSpeed;
	bool m_isSprinting;
	float m_progress;
	bool m_isMoving;

	void StartMove(const sf::Vector2i& newGridPos);
};

inline eOrientation GetOrientationFromDirection(const GridMovementComponent::eDirection direction)
{
	switch (direction)
	{
	case GridMovementComponent::eDirection::North:
		return eOrientation::Up;
	case GridMovementComponent::eDirection::South:
		return eOrientation::Down;
	case GridMovementComponent::eDirection::West:
		return eOrientation::Left;
	case GridMovementComponent::eDirection::East:
		return eOrientation::Right;
	default:
		break;
	}

	return eOrientation::Down;
}

inline GridMovementComponent::eDirection GetDirectionFromOrientation(const eOrientation orientation)
{
	switch (orientation)
	{
	case eOrientation::Up:
		return GridMovementComponent::eDirection::North;
	case eOrientation::Down:
		return GridMovementComponent::eDirection::South;
	case eOrientation::Left:
		return GridMovementComponent::eDirection::West;
	case eOrientation::Right:
		return GridMovementComponent::eDirection::East;
	default:
		break;
	}

	return GridMovementComponent::eDirection::None;
}

#endif
