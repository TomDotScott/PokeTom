#include "Game.h"
#include <fstream>
#include <set>
#include <SFML/Graphics.hpp>

#include "../Engine/TextureManager.h"
#include "../Engine/Globals.h"
#include "../Engine/Timer.h"
#include "../Engine/Input/Keyboard.h"
#include "../Libs/hoxml.h"


Game::Game()
{
}

Game::~Game() = default;

void Game::Update()
{
}

void Game::Render(sf::RenderWindow& window) const
{
	// Draw the background items first
	UIMANAGER.RenderBackground(window);

	// Then the midground stuff
	UIMANAGER.RenderMidground(window);

	// Then, in front of everything, the foreground stuff
	UIMANAGER.RenderForeground(window);

#if !BUILD_MASTER
	DrawText(window, sf::Vector2f{ 0, 10 }, 30, "%.1fFPS", Timer::Get().Fps());
#endif
}
