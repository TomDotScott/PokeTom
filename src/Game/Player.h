#ifndef PLAYER_H
#define PLAYER_H
#include "GridMovementComponent.h"
#include "Level.h"
#include "../Engine/Gameobject.h"
#include "../Engine/Updateable.h"
#include "../Engine/Input/InputMapper.h"

class Player final : public GameObject
{
public:
	Player();

	void Update(float deltaTime);

	void SetPosition(float x, float y) override;
	void SetPosition(const sf::Vector2f& position) override;

	sf::Vector2i GetGridPosition() const;

	void SetLevel(const Level* level);

	enum eAnimationState
	{
		IDLE_UP,
		IDLE_DOWN,
		IDLE_LEFT,
		IDLE_RIGHT,

		WALK_UP,
		WALK_DOWN,
		WALK_LEFT,
		WALK_RIGHT,
	};
	eAnimationState GetAnimationStateFromMovement() const;
	static std::string GetAnimationName(eAnimationState state);

private:
	GridMovementComponent m_movement;
	const Level* m_currentLevel;
	eAnimationState m_animationState;

	enum eInputs : uint8_t
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
	};
	InputMapper m_mapper;

	void Move(GridMovementComponent::eDirection direction);
};

#endif
