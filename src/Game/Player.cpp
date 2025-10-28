#include "Player.h"

#include "../Engine/Globals.h"
#include "../Engine/Input/Keyboard.h"
#include "../Engine/Input/Mouse.h"
#include "../Engine/Timer.h"


Player::Player() :
	GameObject()
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

	constexpr float speed = 32.f * 5;

	if (m_mapper.IsButtonDown(UP))
	{
		m_position.y -= speed * deltaTime;
	}
	else if (m_mapper.IsButtonDown(DOWN))
	{
		m_position.y += speed * deltaTime;
	}

	if (m_mapper.IsButtonDown(LEFT))
	{
		m_position.x -= speed * deltaTime;
	}
	else if (m_mapper.IsButtonDown(RIGHT))
	{
		m_position.x += speed * deltaTime;
	}
}
