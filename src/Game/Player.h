#ifndef PLAYER_H
#define PLAYER_H
#include "GridMovementComponent.h"
#include "../Engine/Gameobject.h"
#include "../Engine/Updateable.h"
#include "../Engine/Input/InputMapper.h"

class Player final : public GameObject, Updateable
{
public:
	Player();

	void Update(float deltaTime) override;

	void SetPosition(float x, float y) override;
	void SetPosition(const sf::Vector2f& position) override;

private:
	GridMovementComponent m_movement;

	enum eInputs : uint8_t
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
	};
	InputMapper m_mapper;
};

#endif
