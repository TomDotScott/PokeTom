#ifndef PLAYER_H
#define PLAYER_H
#include "../Engine/Entity.h"
#include "../Engine/Input/InputMapper.h"
#include "../Engine/GridMovementComponent.h"

class EntityRegistry;
class WorldDefinition;

class Player final : public Entity
{
public:
	Player(EntityRegistry& entities, WorldDefinition& world, const sf::Vector2f& position);

	void Update(float deltaTime) override;
	
	// TODO: This seemed like a good idea last year when I made it, but it's becoming 
	// super difficult to work with now I have more than just the player as an entity that can move in the world!
	void SetCanMoveCallback(const can_move_func& callback);

	void AddOnInteractPressedCallback(const std::function<void()>& callback);

private:
	enum eInputs : uint8_t
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		SPRINT,
		INTERACT,
		ADD_FOLLOWER
	};
	InputMapper m_mapper;
	entity_id_t m_followerEntityID;

	// TODO: These should have a better mechanism to access
	WorldDefinition& m_world;
	EntityRegistry& m_entities;

	void Move();

	void SpawnFollowerEntity();
	void RemoveFollowerEntity();
};

#endif
