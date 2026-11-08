#include "PathfindComponent.h"

#include <cmath>
#include <queue>
#include <unordered_map>
#include <type_traits>
#include <vector>
#include <utility>

#include "../Engine/Asserts.h"
#include "../Engine/Entity.h"
#include "../Engine/GridMovementComponent.h"

namespace
{
	struct Vec2iHash
	{
		size_t operator()(const sf::Vector2i& v) const
		{
			return std::hash<long long>()((static_cast<long long>(v.x) << 32) ^ static_cast<unsigned int>(v.y));
		}
	};

	struct PathNode
	{
		sf::Vector2i m_Pos;
		int m_F;
		bool operator>(const PathNode& o) const
		{
			return m_F > o.m_F;
		}
	};

	int ManHattanDist(const sf::Vector2i& a, const sf::Vector2i& b)
	{
		int absX = std::abs(a.x - b.x);
		int absY = std::abs(a.y - b.y);
		return absX + absY;
	}
}

PathfindComponent::PathfindComponent(Entity* owner, IsWalkableFn fn, int maxSearchNodes) :
	m_owner(owner),
	m_gmc(nullptr),
	m_isWalkable(std::move(fn)),
	m_maxSearchNodes(maxSearchNodes),
	m_hasTarget(false),
	m_pathDirty(false)
{
	if (!owner->HasComponent<GridMovementComponent>())
	{
		m_owner->AddComponent<GridMovementComponent>(owner, GRID_SIZE, ENTITY_WALK_SPEED, ENTITY_RUN_SPEED);
	}

	m_gmc = m_owner->GetComponent<GridMovementComponent>();
	ASSERT(m_gmc);
}

void PathfindComponent::SetTarget(const sf::Vector2i& gridPos)
{
	if (m_hasTarget && m_target == gridPos)
	{
		return;
	}

	m_target = gridPos;
	m_hasTarget = true;
	m_pathDirty = true;
}

void PathfindComponent::ClearTarget()
{
	m_path.clear();
	m_hasTarget = false;
}

void PathfindComponent::SetSprinting(bool sprinting)
{
	m_gmc->SetSprinting(sprinting);
}

sf::Vector2i PathfindComponent::GetGridPosition() const
{
	const sf::Vector2f p = m_gmc->GetWorldPosition();
	return sf::Vector2i{
		static_cast<int>(std::floor(p.x / GRID_SIZE)),
		static_cast<int>(std::floor(p.y / GRID_SIZE))
	};
}

bool PathfindComponent::HasReachedTarget() const
{
	return m_hasTarget && m_gmc->CanMove() && GetGridPosition() == m_target;
}

void PathfindComponent::Update(const float)
{
	if (!m_hasTarget || !m_gmc->CanMove())
	{
		return;
	}

	const sf::Vector2i current = GetGridPosition();
	if (current == m_target)
	{
		m_path.clear();
		return;
	}

	if (m_pathDirty || m_path.empty())
	{
		m_path = FindPath(current, m_target);
		m_pathDirty = false;
		if (m_path.empty())
		{
			return;
		}
	}

	const sf::Vector2i next = m_path.front();
	const sf::Vector2i delta = next - current;
	if (std::abs(delta.x) + std::abs(delta.y) != 1)
	{
		m_pathDirty = true; // drifted off path
		return;
	}

	const eOrientation orientation = GetOrientationFromVector(sf::Vector2f(static_cast<float>(delta.x), static_cast<float>(delta.y)));
	const eDirection direction = GetDirectionFromOrientation(orientation);

	if (m_gmc->CanMove(direction))
	{
		m_gmc->Move(orientation);
		m_path.pop_front();
	}
	else
	{
		m_pathDirty = true; // blocked, replan next frame
	}
}

std::deque<sf::Vector2i> PathfindComponent::FindPath(const sf::Vector2i& from, const sf::Vector2i& to) const
{
	static const sf::Vector2i kDirs[4] = {
		{0, -1},
		{1, 0},
		{0, 1},
		{-1, 0}
	};

	std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode>> open;
	std::unordered_map<sf::Vector2i, sf::Vector2i, Vec2iHash> cameFrom;
	std::unordered_map<sf::Vector2i, int, Vec2iHash> cost;

	open.push({ from, ManHattanDist(from, to) });
	cost[from] = 0;
	cameFrom[from] = from;

	int expanded = 0;
	while (!open.empty() && expanded++ < m_maxSearchNodes)
	{
		const sf::Vector2i cur = open.top().m_Pos;
		open.pop();

		if (cur == to)
		{
			std::deque<sf::Vector2i> path;
			for (sf::Vector2i p = to; p != from; p = cameFrom[p])
			{
				path.push_front(p);
			}
			return path;
		}

		for (const sf::Vector2i& d : kDirs)
		{
			const sf::Vector2i n = cur + d;

			// Always allow the goal tile so a target on an "occupied" tile still resolves
			if (n != to && !m_isWalkable(n))
			{
				continue;
			}

			const int newCost = cost[cur] + 1;
			const auto it = cost.find(n);
			if (it == cost.end() || newCost < it->second)
			{
				cost[n] = newCost;
				cameFrom[n] = cur;
				open.push({ n, newCost + ManHattanDist(n, to) });
			}
		}
	}

	return {};
}


