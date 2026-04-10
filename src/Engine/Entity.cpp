#include "Entity.h"

#include "CodeGen/Resources.hpp"

EntityAnimation::EntityAnimation(const std::string_view& animationPath, const eAnimationName initialAnimation) :
	AnimationName{ initialAnimation },
	Player{ AnimationDictionary::Create(animationPath) }
{
}

void EntityAnimation::PlayAnimation(const bool forceRestart)
{
	Player.PlayAnimation(GetAnimationName(AnimationName), forceRestart);
}

std::string_view EntityAnimation::GetAnimationName(const eAnimationName state)
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
	}

	return "UNKNOWN";
}

Entity::Entity(const WorldDefinition* const gameWorld, EntityAnimation animation) :
	m_world{ gameWorld },
	m_movement{ 32.f, 3.f, 7.5f },
	m_animation{ std::move(animation) }
{
}

void Entity::Update(const float deltaTime)
{
	UpdateInternal(deltaTime);
}

const AnimationPlayer& Entity::GetAnimator() const
{
	return m_animation.Player;
}

void Entity::SetPosition(const float x, const float y)
{
	SetPosition({ x, y });
}

void Entity::SetPosition(const sf::Vector2f& position)
{
	GameObject::SetPosition(position);
	m_movement.SetWorldPosition(position);
}

void Entity::SetOrientation(const eOrientation orientation)
{
	m_movement.SetDirection(GetDirectionFromOrientation(orientation));
	m_animation.PlayAnimation(false);
}


// TODO: This could do with a refactor to have a state machine
eOrientation Entity::GetCurrentOrientation() const
{
	if (m_movement.IsMoving())
	{
		return GetOrientationFromDirection(m_movement.GetCurrentDirection());
	}

	if (m_movement.GetPreviousDirection() == GridMovementComponent::eDirection::None)
	{
		return GetOrientationFromDirection(m_movement.GetCurrentDirection());
	}

	return GetOrientationFromDirection(m_movement.GetPreviousDirection());
}

void Entity::UpdateInternal(const float deltaTime)
{
	m_position = m_movement.GetWorldPosition();

	const eOrientation orientation = GetCurrentOrientation();
	if (m_movement.WasMoving())
	{
		m_animation.AnimationName = m_movement.IsSprinting()
										? GetSprintAnimation(orientation)
										: GetWalkAnimation(orientation);
	}
	else
	{
		m_animation.AnimationName = GetIdleAnimation(orientation);
	}

	m_animation.PlayAnimation(false);
	m_animation.Player.Update(deltaTime);
}

void Entity::Move(const GridMovementComponent::eDirection direction)
{
	if (CanMove(direction))
	{
		m_movement.Move(direction);
	}
	else
	{
		m_movement.SetDirection(direction);
	}
}

bool Entity::CanMove(const GridMovementComponent::eDirection direction) const
{
	const sf::Vector2f newPosition = GetNewGridPosition(direction);

	const auto& currentLevel = m_world->GetLevelAtPosition(newPosition);
	return currentLevel != nullptr && currentLevel->CanMoveTo(newPosition);
}

sf::Vector2f Entity::GetNewGridPosition(const GridMovementComponent::eDirection direction) const
{
	sf::Vector2f moveDirection(0, 0);

	switch (direction)
	{
	case GridMovementComponent::eDirection::North:
		moveDirection.y -= 1;
		break;
	case GridMovementComponent::eDirection::South:
		moveDirection.y += 1;
		break;
	case GridMovementComponent::eDirection::West:
		moveDirection.x -= 1;
		break;
	case GridMovementComponent::eDirection::East:
		moveDirection.x += 1;
		break;
	case GridMovementComponent::eDirection::None:
		break;
	}

	const sf::Vector2f newPosition = GetPosition() + moveDirection * 32.f;

	return {
		std::round(newPosition.x / 32.f) * 32.f,
		std::round(newPosition.y / 32.f) * 32.f
	};
}

EntityAnimation::eAnimationName Entity::GetWalkAnimation(eOrientation orientation)
{
	switch (orientation)
	{
	case eOrientation::Up:
		return EntityAnimation::WALK_UP;
	case eOrientation::Down:
		return EntityAnimation::WALK_DOWN;
	case eOrientation::Left:
		return EntityAnimation::WALK_LEFT;
	case eOrientation::Right:
		return EntityAnimation::WALK_RIGHT;
	}

	return EntityAnimation::WALK_DOWN;
}

EntityAnimation::eAnimationName Entity::GetIdleAnimation(eOrientation orientation)
{
	switch (orientation)
	{
	case eOrientation::Up:
		return EntityAnimation::IDLE_UP;
	case eOrientation::Down:
		return EntityAnimation::IDLE_DOWN;
	case eOrientation::Left:
		return EntityAnimation::IDLE_LEFT;
	case eOrientation::Right:
		return EntityAnimation::IDLE_RIGHT;
	}

	return EntityAnimation::IDLE_DOWN;
}

EntityAnimation::eAnimationName Entity::GetSprintAnimation(eOrientation orientation)
{
	switch (orientation)
	{
	case eOrientation::Up:
		return EntityAnimation::RUN_UP;
	case eOrientation::Down:
		return EntityAnimation::RUN_DOWN;
	case eOrientation::Left:
		return EntityAnimation::RUN_LEFT;
	case eOrientation::Right:
		return EntityAnimation::RUN_RIGHT;
	}

	return EntityAnimation::RUN_DOWN;
}
