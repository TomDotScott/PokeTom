#ifndef GRIDMOVEMENTCOMPONENT_H
#define GRIDMOVEMENTCOMPONENT_H
#include <functional>
#include <SFML/System/Vector2.hpp>

#include "Entity.h"
#include "../Engine/Orientation.h"
#include "../Engine/IUpdateable.h"

enum class eDirection
{
	None = -1,
	North,
	South,
	West,
	East
};

typedef std::function<bool(eDirection dir)> can_move_func;

class GridMovementComponent : public IUpdateable
{
public:
	GridMovementComponent(Entity* owner, float tileSize, float walkTilesPerSecond, float sprintTilesPerSecond);

	void Move(eDirection direction);

	void SetDirection(eDirection direction);

	eDirection GetCurrentDirection() const;
	eDirection GetPreviousDirection() const;

	void Update(float deltaTime) override;

	const sf::Vector2f& GetWorldPosition() const;
	void SetWorldPosition(const sf::Vector2f& pos);

	bool IsMoving() const;

	// Updates one frame behind IsMoving()
	bool WasMoving() const;

	void SetSprinting(bool isSprinting);
	bool IsSprinting() const;

	eOrientation GetCurrentOrientation() const;

	void SetCanMoveCallback(can_move_func callback);

private:
	Entity* m_owningEntity;
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

	// Updates one frame behind
	bool m_wasMoving;

	bool m_isMoving;

	can_move_func m_canMove;

	void StartMove(const sf::Vector2i& dir);
};

inline eOrientation GetOrientationFromDirection(const eDirection direction)
{
	switch (direction)
	{
	case eDirection::North:
		return eOrientation::Up;
	case eDirection::South:
		return eOrientation::Down;
	case eDirection::West:
		return eOrientation::Left;
	case eDirection::East:
		return eOrientation::Right;
	default:
		break;
	}

	return eOrientation::Down;
}

inline eDirection GetDirectionFromOrientation(const eOrientation orientation)
{
	switch (orientation)
	{
	case eOrientation::Up:
		return eDirection::North;
	case eOrientation::Down:
		return eDirection::South;
	case eOrientation::Left:
		return eDirection::West;
	case eOrientation::Right:
		return eDirection::East;
	default:
		break;
	}

	return eDirection::None;
}

#endif
