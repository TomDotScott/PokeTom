#include "AnimationComponent.h"
#include "../Entity.h"
#include "../CodeGen/Resources.hpp"

EntityAnimationComponent::EntityAnimationComponent(
	Entity* owner,
	const std::string_view& animDictResourceName,
	const eAnimationName initialAnimation
) :
	m_owningEntity(owner),
	m_animationName{ initialAnimation },
	m_player{ AnimationDictionary::Create(GET_ANIMATION_PATH(animDictResourceName)) }
{
	m_player.PlayAnimation(GetAnimationName(m_animationName), true);
}

void EntityAnimationComponent::Update(const float deltaTime)
{
	m_player.Update(deltaTime);
}

void EntityAnimationComponent::PlayAnimation(const eAnimationName animation, const bool forceRestart)
{
	m_animationName = animation;
	m_player.PlayAnimation(GetAnimationName(m_animationName), forceRestart);
}

const AnimationPlayer& EntityAnimationComponent::GetAnimator() const
{
	return m_player;
}

hash_type EntityAnimationComponent::GetAnimationName(const eAnimationName state)
{
	switch (state)
	{
	case IDLE_UP:
		return HASH("idle_up");
	case IDLE_DOWN:
		return HASH("idle_down");
	case IDLE_LEFT:
		return HASH("idle_left");
	case IDLE_RIGHT:
		return HASH("idle_right");
	case WALK_UP:
		return HASH("walk_up");
	case WALK_DOWN:
		return HASH("walk_down");
	case WALK_LEFT:
		return HASH("walk_left");
	case WALK_RIGHT:
		return HASH("walk_right");
	case RUN_UP:
		return HASH("run_up");
	case RUN_DOWN:
		return HASH("run_down");
	case RUN_LEFT:
		return HASH("run_left");
	case RUN_RIGHT:
		return HASH("run_right");
	case CYCLE_UP:
		return HASH("cycle_up");
	case CYCLE_DOWN:
		return HASH("cycle_down");
	case CYCLE_LEFT:
		return HASH("cycle_left");
	case CYCLE_RIGHT:
		return HASH("cycle_right");
	case BATTLE_BACK:
		return HASH("battle_back");
	case BATTLE_FRONT:
		return HASH("battle_front");
	}

	return HASH("UNKNOWN");
}

EntityAnimationComponent::eAnimationName EntityAnimationComponent::GetWalkAnimation(const eOrientation orientation)
{
	switch (orientation)
	{
	case eOrientation::Up:
		return WALK_UP;
	case eOrientation::Down:
		return WALK_DOWN;
	case eOrientation::Left:
		return WALK_LEFT;
	case eOrientation::Right:
		return WALK_RIGHT;
	}

	return WALK_DOWN;
}

EntityAnimationComponent::eAnimationName EntityAnimationComponent::GetCycleAnimation(const eOrientation orientation)
{
	switch (orientation)
	{
	case eOrientation::Up:
		return CYCLE_UP;
	case eOrientation::Down:
		return CYCLE_DOWN;
	case eOrientation::Left:
		return CYCLE_LEFT;
	case eOrientation::Right:
		return CYCLE_RIGHT;
	}

	return CYCLE_DOWN;
}

EntityAnimationComponent::eAnimationName EntityAnimationComponent::GetIdleAnimation(const eOrientation orientation)
{
	switch (orientation)
	{
	case eOrientation::Up:
		return IDLE_UP;
	case eOrientation::Down:
		return IDLE_DOWN;
	case eOrientation::Left:
		return IDLE_LEFT;
	case eOrientation::Right:
		return IDLE_RIGHT;
	}

	return IDLE_DOWN;
}

EntityAnimationComponent::eAnimationName EntityAnimationComponent::GetSprintAnimation(const eOrientation orientation)
{
	switch (orientation)
	{
	case eOrientation::Up:
		return RUN_UP;
	case eOrientation::Down:
		return RUN_DOWN;
	case eOrientation::Left:
		return RUN_LEFT;
	case eOrientation::Right:
		return RUN_RIGHT;
	}

	return RUN_DOWN;
}
