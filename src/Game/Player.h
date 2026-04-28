#ifndef PLAYER_H
#define PLAYER_H
#include "../Engine/Entity.h"
#include "../Engine/Input/InputMapper.h"
#include "../Engine/GridMovementComponent.h"

class Player final : public Entity
{
public:
	Player();
	Player(const sf::Vector2f& position);

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
	};
	InputMapper m_mapper;

	void Move();
};

#endif
