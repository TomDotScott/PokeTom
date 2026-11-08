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

void EntityFollowComponent::Update(const float deltaTime)
{
	Entity* target = m_entityRegistry.Get(m_targetEntityID);
	if (!target)
	{
		return;
	}

	ASSERT(m_ownerGMC);

	const GridMovementComponent* targetGMC = target->GetComponent<GridMovementComponent>();
	ASSERT(targetGMC);

	m_ownerGMC->SetSprinting(true);

	if (targetGMC->JustCompletedMovement())
	{
		m_trail.push_back(targetGMC->GetStartPosition());
	}

	if (targetGMC->IsMoving())
	{
		m_trail.push_back(targetGMC->GetStartPosition());
	}

	if (!m_trail.empty())
	{
		const sf::Vector2f targetTilePos = m_trail.front();
		const sf::Vector2f currentPos = m_ownerGMC->GetWorldPosition();

		if (targetTilePos == currentPos)
		{
			m_trail.pop_front();
			return;
		}


		if (m_ownerGMC->CanMove())
		{
			const sf::Vector2f d = targetTilePos - currentPos;
			const eOrientation orientation = GetOrientationFromVector(d);
			const eDirection direction = GetDirectionFromOrientation(orientation);

			if (m_ownerGMC->CanMove(direction))
			{
				m_ownerGMC->Move(orientation);
				m_trail.pop_front();
			}
		}
	}
}
