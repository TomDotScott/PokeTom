#ifndef FOLLOWENTITYCOMPONENT_H
#define FOLLOWENTITYCOMPONENT_H

#include <deque>

#include "../Engine/IUpdateable.h"
#include "../Engine/Entity.h"


class EntityRegistry;
class GridMovementComponent;

class EntityFollowComponent : public IUpdateable
{
public:
	EntityFollowComponent(Entity* owner, EntityRegistry& entities, entity_id_t targetEntity);
	void Update(float deltaTime) override;

private:
	Entity* m_owner;
	entity_id_t m_targetEntityID;

	// TODO: Is this a good idea? MonsterPartyComponent does something similar - is it worth just making GameContext globally accessible?
	EntityRegistry& m_entityRegistry;

	GridMovementComponent* m_ownerGMC;

	std::deque<sf::Vector2f> m_trail;
	bool m_targetWasMoving;
};

#endif // FOLLOWENTITYCOMPONENT_H
