#include "AnimationComponent.h"
#include "../Entity.h"
#include "../Asserts.h"
#include "../GridMovementComponent.h"
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
	const GridMovementComponent* movement = m_owningEntity->GetComponent<GridMovementComponent>();
	ASSERT_MSG(movement != nullptr, "Entity does not have GridMovementComponent attached!");

	const eOrientation orientation = movement->GetCurrentOrientation();
	if (movement->IsCycling())
	{
		PlayAnimation(GetCycleAnimation(orientation), false);
	}
	else if (movement->WasMoving())
	{
		PlayAnimation(movement->IsSprinting()
			? GetSprintAnimation(orientation)
			: GetWalkAnimation(orientation), false);
	}
	else
	{
		PlayAnimation(GetIdleAnimation(orientation), false);
	}


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

std::string_view EntityAnimationComponent::GetAnimationName(const eAnimationName state)
{
	switch (state)
	{
	case IDLE_UP:
		return "idle_up";
	case IDLE_DOWN:
		return "idle_down";
	case IDLE_LEFT:
		return "idle_left";
	case IDLE_RIGHT:
		return "idle_right";
	case WALK_UP:
		return "walk_up";
	case WALK_DOWN:
		return "walk_down";
	case WALK_LEFT:
		return "walk_left";
	case WALK_RIGHT:
		return "walk_right";
	case RUN_UP:
		return "run_up";
	case RUN_DOWN:
		return "run_down";
	case RUN_LEFT:
		return "run_left";
	case RUN_RIGHT:
		return "run_right";
	case CYCLE_UP:
		return "cycle_up";
	case CYCLE_DOWN:
		return "cycle_down";
	case CYCLE_LEFT:
		return "cycle_left";
	case CYCLE_RIGHT:
		return "cycle_right";
	}

	return "UNKNOWN";
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
