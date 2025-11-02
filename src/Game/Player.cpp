#include "Player.h"

#include "../Engine/Globals.h"
#include "../Engine/Input/Keyboard.h"
#include "../Engine/Input/Mouse.h"
#include "../Engine/Timer.h"


Player::Player() :
	GameObject(),
	m_movement(32.f, 5.f),
	m_currentLevel(nullptr),
	m_animationState(IDLE_DOWN),
	m_animationPlayer(AnimationDictionary::Create("animation\\player_boy.xml"))
{
	m_mapper.Map(UP, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::W));
	m_mapper.Map(DOWN, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::S));

	m_mapper.Map(LEFT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::A));
	m_mapper.Map(RIGHT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::D));
}

void Player::Update(const float deltaTime)
{
	// Update inputs
	m_mapper.Update();

	if (!m_movement.IsMoving())
	{
		if (m_mapper.IsButtonDown(UP))
		{
			Move(GridMovementComponent::eDirection::Up);
		}
		else if (m_mapper.IsButtonDown(DOWN))
		{
			Move(GridMovementComponent::eDirection::Down);
		}
		else if (m_mapper.IsButtonDown(LEFT))
		{
			Move(GridMovementComponent::eDirection::Left);
		}
		else if (m_mapper.IsButtonDown(RIGHT))
		{
			Move(GridMovementComponent::eDirection::Right);
		}
	}

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

void Player::SetLevel(const Level* level)
{
	m_currentLevel = level;
}

int Player::GetZIndex() const
{
	return m_currentLevel->GetPlayerZIndex();
}

Player::eAnimationState Player::GetAnimationStateFromMovement() const
{
	if (m_movement.IsMoving())
	{
		switch (m_movement.GetCurrentDirection())
		{
		case GridMovementComponent::eDirection::None:
			return IDLE_DOWN;
		case GridMovementComponent::eDirection::Up:
			return WALK_UP;
		case GridMovementComponent::eDirection::Down:
			return WALK_DOWN;
		case GridMovementComponent::eDirection::Left:
			return WALK_LEFT;
		case GridMovementComponent::eDirection::Right:
			return WALK_RIGHT;
		}
	}
	else
	{
		switch (m_movement.GetPreviousDirection())
		{
		case GridMovementComponent::eDirection::None:
			return IDLE_DOWN;
		case GridMovementComponent::eDirection::Up:
			return IDLE_UP;
		case GridMovementComponent::eDirection::Down:
			return IDLE_DOWN;
		case GridMovementComponent::eDirection::Left:
			return IDLE_LEFT;
		case GridMovementComponent::eDirection::Right:
			return IDLE_RIGHT;
		}
	}

	return IDLE_DOWN;
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
	case GridMovementComponent::eDirection::Up:
		newGridPosition.y -= 1;
		break;
	case GridMovementComponent::eDirection::Down:
		newGridPosition.y += 1;
		break;
	case GridMovementComponent::eDirection::Left:
		newGridPosition.x -= 1;
		break;
	case GridMovementComponent::eDirection::Right:
		newGridPosition.x += 1;
		break;
	case GridMovementComponent::eDirection::None:
		break;
	}

	if (m_currentLevel->CanMoveTo(newGridPosition.x, newGridPosition.y))
	{
		m_movement.Move(direction);
	}
}
