#include "Player.h"

#include "../Engine/Globals.h"
#include "../Engine/Input/Keyboard.h"
#include "../Engine/Input/Mouse.h"
#include "../Engine/Timer.h"


Player::Player() :
	GameObject(),
	m_movement(32.f, 3.f, 7.5f),
	m_currentLevel(nullptr),
	m_animationState(IDLE_DOWN),
	m_animationPlayer(AnimationDictionary::Create("animation\\player_boy.xml"))
{
	m_mapper.Map(UP, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::W));
	m_mapper.Map(DOWN, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::S));

	m_mapper.Map(LEFT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::A));
	m_mapper.Map(RIGHT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::D));

	m_mapper.Map(SPRINT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::LShift));
}

void Player::Update(const float deltaTime)
{
	// Update inputs
	m_mapper.Update();

	if (!m_movement.IsMoving())
	{
		if (m_mapper.IsButtonDown(UP))
		{
			Move(GridMovementComponent::eDirection::North);
		}
		else if (m_mapper.IsButtonDown(DOWN))
		{
			Move(GridMovementComponent::eDirection::South);
		}
		else if (m_mapper.IsButtonDown(LEFT))
		{
			Move(GridMovementComponent::eDirection::West);
		}
		else if (m_mapper.IsButtonDown(RIGHT))
		{
			Move(GridMovementComponent::eDirection::East);
		}
	}

	m_movement.SetSprinting(m_mapper.IsButtonDown(SPRINT));

	m_position = m_movement.GetWorldPosition();

	m_animationPlayer.PlayAnimation(GetAnimationName(GetAnimationStateFromMovement()), false);
	m_animationPlayer.Update(deltaTime);
}

void Player::SetPosition(const float x, const float y)
{
	SetPosition({ x, y });
}

void Player::SetPosition(const sf::Vector2f& position)
{
	GameObject::SetPosition(position);

	m_movement.SetGridPosition({ static_cast<int>(position.x) / 32, static_cast<int>(position.y) / 32 });
}

sf::Vector2i Player::GetGridPosition() const
{
	return m_movement.GetGridPosition();
}

void Player::SetGridPosition(const sf::Vector2i& gridPosition)
{
	m_movement.SetGridPosition(gridPosition);
	SetPosition(m_movement.GetWorldPosition());
}

void Player::SetLevel(const std::shared_ptr<Level>& level)
{
	m_currentLevel = level;
}

int Player::GetZIndex() const
{
	return m_currentLevel->GetPlayerZIndex();
}

void Player::SetOrientation(const eOrientation orientation)
{
	m_movement.SetDirection(GetDirectionFromOrientation(orientation));
	m_animationPlayer.PlayAnimation(GetAnimationName(GetAnimationStateFromMovement()), false);
}

// TODO: This could do with a refactor to have a PlayerState machine
eOrientation Player::GetCurrentOrientation() const
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

Player::eAnimationState Player::GetAnimationStateFromMovement() const
{
	const eOrientation orientation = GetCurrentOrientation();

	if (m_movement.IsMoving())
	{
		if (m_movement.IsSprinting()) {
			return GetSprintAnimationState(orientation);
		}

		return GetWalkAnimationState(orientation);
	}

	return GetIdleAnimationState(orientation);
}

std::string Player::GetAnimationName(const eAnimationState state)
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

const AnimationPlayer& Player::GetAnimator() const
{
	return m_animationPlayer;
}

void Player::Move(const GridMovementComponent::eDirection direction)
{
	sf::Vector2i newGridPosition = GetGridPosition();

	switch (direction)
	{
	case GridMovementComponent::eDirection::North:
		newGridPosition.y -= 1;
		break;
	case GridMovementComponent::eDirection::South:
		newGridPosition.y += 1;
		break;
	case GridMovementComponent::eDirection::West:
		newGridPosition.x -= 1;
		break;
	case GridMovementComponent::eDirection::East:
		newGridPosition.x += 1;
		break;
	case GridMovementComponent::eDirection::None:
		break;
	}

	if (m_currentLevel->CanMoveTo(newGridPosition.x, newGridPosition.y))
	{
		m_movement.Move(direction);
	}
	else
	{
		m_movement.SetDirection(direction);
	}
}

Player::eAnimationState Player::GetWalkAnimationState(const eOrientation orientation)
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

Player::eAnimationState Player::GetIdleAnimationState(const eOrientation orientation)
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

Player::eAnimationState Player::GetSprintAnimationState(const eOrientation orientation)
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
