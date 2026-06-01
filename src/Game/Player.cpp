#include "Player.h"

#include "DialogueBox.h"
#include "../Engine/Asserts.h"
#include "../Engine/CodeGen/Resources.hpp"
#include "../Engine/Globals.h"
#include "../Engine/Animation/AnimationComponent.h"
#include "../Engine/Input/Keyboard.h"


Player::Player(const sf::Vector2f& position) :
	Entity(position, { 1.f, 1.f })
{
	AddComponent<GridMovementComponent>(this, 32.f, 3.f, 6.5f);
	AddComponent<EntityAnimationComponent>(this, "PLAYER_BOY", EntityAnimationComponent::eAnimationName::IDLE_DOWN);
	m_mapper.Map(UP, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::W));
	m_mapper.Map(DOWN, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::S));

	m_mapper.Map(LEFT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::A));
	m_mapper.Map(RIGHT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::D));

	m_mapper.Map(SPRINT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::LShift));

	m_mapper.Map(INTERACT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::E));

	SetPosition(position);

	// The player is active by default
	Entity::OnActivate();
}

void Player::Move()
{
	GridMovementComponent* movement = GetComponent<GridMovementComponent>();
	ASSERT_MSG(movement != nullptr, "No GridMovementComponent attached to the Player!");

	if (!movement->IsMoving())
	{
		if (m_mapper.IsButtonDown(UP))
		{
			movement->Move(eDirection::North);
		}
		else if (m_mapper.IsButtonDown(DOWN))
		{
			movement->Move(eDirection::South);
		}
		else if (m_mapper.IsButtonDown(LEFT))
		{
			movement->Move(eDirection::West);
		}
		else if (m_mapper.IsButtonDown(RIGHT))
		{
			movement->Move(eDirection::East);
		}
	}

	movement->SetSprinting(m_mapper.IsButtonDown(SPRINT));
}

void Player::Update(const float deltaTime)
{
	// Update inputs
	m_mapper.Update();

	if (!DialogueBox::IsVisible())
	{
		Move();
	}

	Entity::Update(deltaTime);
}

void Player::SetCanMoveCallback(const can_move_func& callback)
{
	GetComponent<GridMovementComponent>()->SetCanMoveCallback(callback);
}

void Player::AddOnInteractPressedCallback(const std::function<void()>& callback)
{
	m_mapper.OnButtonPressed(INTERACT, callback);
}
