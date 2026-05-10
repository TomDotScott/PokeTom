#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H
#include "../Orientation.h"

#include "../IUpdateable.h"
#include "AnimationPlayer.h"

class Entity;

class EntityAnimationComponent : public IUpdateable
{
public:
	enum eAnimationName
	{
		IDLE_UP = 1,
		IDLE_DOWN = 1 << 2,
		IDLE_LEFT = 1 << 3,
		IDLE_RIGHT = 1 << 4,

		WALK_UP = 1 << 5,
		WALK_DOWN = 1 << 6,
		WALK_LEFT = 1 << 7,
		WALK_RIGHT = 1 << 8,

		RUN_UP = 1 << 9,
		RUN_DOWN = 1 << 10,
		RUN_LEFT = 1 << 11,
		RUN_RIGHT = 1 << 12,

		CYCLE_UP = 1 << 13,
		CYCLE_DOWN = 1 << 14,
		CYCLE_LEFT = 1 << 15,
		CYCLE_RIGHT = 1 << 16,
	};

	EntityAnimationComponent(Entity* owner, const std::string_view& animDictResourceName, eAnimationName initialAnimation);

	void Update(float deltaTime) override;

	void PlayAnimation(eAnimationName animation, bool forceRestart);

	const AnimationPlayer& GetAnimator() const;

	static eAnimationName GetIdleAnimation(eOrientation orientation);
	static eAnimationName GetSprintAnimation(eOrientation orientation);
	static eAnimationName GetWalkAnimation(eOrientation orientation);
	static eAnimationName GetCycleAnimation(eOrientation orientation);

private:
	Entity* m_owningEntity;
	eAnimationName m_animationName;
	AnimationPlayer m_player;

	static hash_type GetAnimationName(eAnimationName state);
};

#endif
