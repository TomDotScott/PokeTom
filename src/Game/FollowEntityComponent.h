#ifndef FOLLOWENTITYCOMPONENT_H
#define FOLLOWENTITYCOMPONENT_H

#include <SFML/System/Vector2.hpp>
#include "../Engine/Entity.h"
#include "../Engine/IUpdateable.h"
#include "PathfindComponent.h"

class EntityRegistry;

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

	PathfindComponent* m_pathfind;

	sf::Vector2i m_behindTile;
	bool m_hasBehindTile;
};

#endif // FOLLOWENTITYCOMPONENT_H
