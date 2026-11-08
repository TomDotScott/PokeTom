#ifndef PATHFINDCOMPONENT_H
#define PATHFINDCOMPONENT_H

#include <functional>
#include <deque>
#include <SFML/System/Vector2.hpp>
#include "../Engine/IUpdateable.h"

class Entity;
class GridMovementComponent;

class PathfindComponent : public IUpdateable
{
public:
	using IsWalkableFn = std::function<bool(const sf::Vector2i&)>;

	PathfindComponent(Entity* owner, IsWalkableFn fn, int maxSearchNodes = 2048);

	void Update(float deltaTime) override;

	void SetTarget(const sf::Vector2i& gridPos);
	void ClearTarget();
	void SetSprinting(bool sprinting);

	bool HasTarget() const;
	bool HasReachedTarget() const;
	sf::Vector2i GetGridPosition() const;

private:
	Entity* m_owner;
	GridMovementComponent* m_gmc;
	IsWalkableFn m_isWalkable;
	int m_maxSearchNodes;

	std::deque<sf::Vector2i> m_path;
	sf::Vector2i m_target;
	bool m_hasTarget;
	bool m_pathDirty;

	std::deque<sf::Vector2i> FindPath(const sf::Vector2i& from, const sf::Vector2i& to) const;
};
#endif
