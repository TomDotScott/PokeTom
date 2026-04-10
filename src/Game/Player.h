#ifndef PLAYER_H
#define PLAYER_H
#include "GridMovementComponent.h"
#include "Level.h"
#include "WorldDefinition.h"
#include "../Engine/Entity.h"
#include "../Engine/Input/InputMapper.h"

class Player final : public Entity
{
public:
	Player(const WorldDefinition* gameWorld);

	void Update(float deltaTime) override;

private:
	enum eInputs : uint8_t
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		SPRINT
	};
	InputMapper m_mapper;
};

#endif
