#ifndef PLAYER_H
#define PLAYER_H
#include "GridMovementComponent.h"
#include "Level.h"
#include "../Engine/Gameobject.h"
#include "../Engine/Animation/AnimationPlayer.h"
#include "../Engine/Input/InputMapper.h"

class Player final : public GameObject
{
public:
	Player();

	void Update(float deltaTime);

	void SetPosition(float x, float y) override;
	void SetPosition(const sf::Vector2f& position) override;

	sf::Vector2i GetGridPosition() const;
	void SetGridPosition(const sf::Vector2i& gridPosition);

	void SetLevel(const std::shared_ptr<Level>& level);
	int GetZIndex() const;

	void SetOrientation(eOrientation);
	eOrientation GetCurrentOrientation() const;

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

		RUN_UP,
		RUN_DOWN,
		RUN_LEFT,
		RUN_RIGHT,
	};
	eAnimationState GetAnimationStateFromMovement() const;
	static std::string GetAnimationName(eAnimationState state);

	const AnimationPlayer& GetAnimator() const;

private:
	GridMovementComponent m_movement;
	std::shared_ptr<Level> m_currentLevel;
	eAnimationState m_animationState;

	AnimationPlayer m_animationPlayer;

	enum eInputs : uint8_t
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		SPRINT
	};
	InputMapper m_mapper;

	void Move(GridMovementComponent::eDirection direction);
	static eAnimationState GetWalkAnimationState(eOrientation orientation);
	static eAnimationState GetIdleAnimationState(eOrientation orientation);
	static eAnimationState GetSprintAnimationState(eOrientation orientation);
};

#endif
