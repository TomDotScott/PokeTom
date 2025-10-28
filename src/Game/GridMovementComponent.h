#ifndef GRIDMOVEMENTCOMPONENT_H
#define GRIDMOVEMENTCOMPONENT_H
#include <SFML/System/Vector2.hpp>

#include "../Engine/Updateable.h"

class GridMovementComponent : Updateable
{
public:
	enum class eDirection
	{
		Up,
		Down,
		Left,
		Right
	};

	GridMovementComponent(float tileSize, float tilesPerSecond);

	void SetGridPosition(const sf::Vector2i& gridPos);
	void Move(eDirection direction);

	void Update(float deltaTime) override;
	const sf::Vector2f& GetWorldPosition() const;

	bool IsMoving() const;
	sf::Vector2i GetGridPosition() const;

private:
	sf::Vector2i m_gridPos;
	sf::Vector2i m_targetGridPos;
	sf::Vector2f m_startPos;
	sf::Vector2f m_endPos;
	sf::Vector2f m_worldPos;

	float m_tileSize;
	float m_speed;
	float m_progress;
	bool m_isMoving;

	void StartMove(const sf::Vector2i& newGridPos);
};

#endif
