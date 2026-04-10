#include "Player.h"

#include "../Engine/CodeGen/Resources.hpp"
#include "../Engine/Globals.h"
#include "../Engine/Input/Keyboard.h"


Player::Player(const WorldDefinition* const gameWorld) :
	Entity(gameWorld, EntityAnimation{ GET_ANIMATION_PATH("PLAYER_BOY"), EntityAnimation::IDLE_DOWN })
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

	UpdateInternal(deltaTime);
}
