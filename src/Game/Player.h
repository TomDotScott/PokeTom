#ifndef PLAYER_H
#define PLAYER_H
#include "../Engine/Entity.h"
#include "../Engine/Input/InputMapper.h"
#include "../Engine/GridMovementComponent.h"

class EntityRegistry;

class Player final : public Entity
{
public:
	Player(EntityRegistry& entities, const sf::Vector2f& position);

	void Update(float deltaTime) override;
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

	EntityRegistry& m_entities;

	void Move();

	void SpawnFollowerEntity();
	void RemoveFollowerEntity();
};

#endif
