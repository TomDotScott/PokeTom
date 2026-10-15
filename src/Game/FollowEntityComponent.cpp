#include "FollowEntityComponent.h"

#include "../Engine/Asserts.h"
#include "../Engine/EntityRegistry.h"
#include "../Engine/GridMovementComponent.h"

EntityFollowComponent::EntityFollowComponent(Entity* owner, EntityRegistry& entities, const entity_id_t targetEntity) :
	m_owner(owner),
	m_targetEntityID(targetEntity),
	m_entityRegistry(entities),
	m_targetWasMoving(false)
{
	if (!m_owner->GetComponent<GridMovementComponent>())
	{
		m_owner->AddComponent<GridMovementComponent>(owner, GRID_SIZE, ENTITY_WALK_SPEED, ENTITY_WALK_SPEED);
	}

	m_ownerGMC = m_owner->GetComponent<GridMovementComponent>();
}

void EntityFollowComponent::Update(float deltaTime)
{
	Entity* target = m_entityRegistry.Get(m_targetEntityID);
	if (!target)
	{
		// TODO: Assert here? Maybe the entity being removed was unintentional
		return;
	}

	ASSERT(m_ownerGMC);
	const GridMovementComponent* targetGMC = target->GetComponent<GridMovementComponent>();
	ASSERT(targetGMC);

	// When the target starts a move, the tile it is leaving becomes our next waypoint
	if (targetGMC->IsMoving() && !m_targetWasMoving)
	{
		m_trail.push_back(targetGMC->GetStartPosition()); // small getter to add
	}

	m_targetWasMoving = targetGMC->IsMoving();

	if (!m_ownerGMC->IsMoving() && !m_trail.empty())
	{
		const sf::Vector2f d = m_trail.front() - m_ownerGMC->GetWorldPosition();
		m_ownerGMC->Move(GetOrientationFromVector(d));
		m_trail.pop_front();
	}
}
