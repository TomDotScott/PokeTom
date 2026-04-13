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

	EntityAnimationComponent(Entity* owner, const std::string_view& animationPath, eAnimationName initialAnimation);

	void Update(float deltaTime) override;

	void PlayAnimation(eAnimationName animation, bool forceRestart);

	const AnimationPlayer& GetAnimator() const;

	static eAnimationName GetIdleAnimation(eOrientation orientation);
	static eAnimationName GetSprintAnimation(eOrientation orientation);
	static eAnimationName GetWalkAnimation(eOrientation orientation);

private:
	Entity* m_owningEntity;
	eAnimationName m_animationName;
	AnimationPlayer m_player;

	static std::string_view GetAnimationName(eAnimationName state);
};

#endif
