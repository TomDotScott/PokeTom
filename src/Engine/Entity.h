#ifndef ENTITY_H
#define ENTITY_H
#include "../Game/GridMovementComponent.h"
#include "../Game/WorldDefinition.h"
#include "Animation/AnimationPlayer.h"
#include "Gameobject.h"


struct EntityAnimation
{
	enum eAnimationName
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

	EntityAnimation(const std::string_view& animationPath, eAnimationName initialAnimation);
	void PlayAnimation(bool forceRestart);

	eAnimationName AnimationName;
	AnimationPlayer Player;

	static std::string_view GetAnimationName(eAnimationName state);
};


class Entity : public GameObject
{
public:
	Entity(const WorldDefinition* gameWorld, EntityAnimation animation);

	virtual void Update(float deltaTime);

	const AnimationPlayer& GetAnimator() const;

	void SetPosition(float x, float y) override;
	void SetPosition(const sf::Vector2f& position) override;

	void SetOrientation(eOrientation orientation);
	eOrientation GetCurrentOrientation() const;

	void Move(GridMovementComponent::eDirection direction);

protected:
	// Needs to be called in Update() in all children. TODO: is there a way to ensure this?
	void UpdateInternal(float deltaTime);

	const WorldDefinition* const m_world;
	GridMovementComponent m_movement;

	EntityAnimation m_animation;

private:
	static EntityAnimation::eAnimationName GetWalkAnimation(eOrientation orientation);
	static EntityAnimation::eAnimationName GetIdleAnimation(eOrientation orientation);
	static EntityAnimation::eAnimationName GetSprintAnimation(eOrientation orientation);
};

#endif
