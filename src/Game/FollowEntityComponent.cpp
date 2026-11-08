#include "FollowEntityComponent.h"

#include "../Engine/Asserts.h"
#include "../Engine/EntityRegistry.h"
#include "../Engine/GridMovementComponent.h"
#include "../Engine/Entity.h"
#include "PathfindComponent.h"


EntityFollowComponent::EntityFollowComponent(Entity* owner, EntityRegistry& entities, const entity_id_t targetEntity) :
	m_owner(owner),
	m_targetEntityID(targetEntity),
	m_entityRegistry(entities),
	m_pathfind(nullptr),
	m_behindTile(),
	m_hasBehindTile(false)
{
	ASSERT(m_owner->HasComponent<PathfindComponent>());
	m_pathfind = m_owner->GetComponent<PathfindComponent>();
}

void EntityFollowComponent::Update(const float deltaTime)
{
	Entity* target = m_entityRegistry.Get(m_targetEntityID);
	if (!target)
	{
		return;
	}

	GridMovementComponent* myGMC = m_owner->GetComponent<GridMovementComponent>();
	ASSERT(myGMC);

	const GridMovementComponent* targetGMC = target->GetComponent<GridMovementComponent>();
	ASSERT(targetGMC);

	myGMC->SetSprinting(targetGMC->IsSprinting());

	// The tile the target just left is, by definition, behind it
	if (targetGMC->IsMoving() || targetGMC->JustCompletedMovement())
	{
		const sf::Vector2f s = targetGMC->GetStartPosition();
		m_behindTile = { static_cast<int>(std::floor(s.x / GRID_SIZE)), static_cast<int>(std::floor(s.y / GRID_SIZE)) };
		m_hasBehindTile = true;
	}

	if (m_hasBehindTile)
	{
		m_pathfind->SetTarget(m_behindTile);
	}
}
