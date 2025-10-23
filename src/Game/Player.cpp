#include "Player.h"

#include "../Engine/Globals.h"
#include "../Engine/Input/Keyboard.h"
#include "../Engine/Input/Mouse.h"
#include "../Engine/Timer.h"


Player::Player() :
	GameObject()
{
}

void Player::Update()
{
	// Update inputs
	m_mapper.Update();
}
