#include "Game.h"
#include <fstream>
#include <iostream>
#include <set>
#include <SFML/Graphics.hpp>
#include "../Engine/TextureManager.h"
#include "../Engine/Globals.h"
#include "../Engine/Maths.h"
#include "../Engine/Timer.h"
#include "../Engine/Animation/AnimDef.h"
#include "../Engine/Input/Keyboard.h"

const std::filesystem::path START_LEVEL = "tiled_export\\player_bedroom.tmj";

Game::Game() :
	Updateable(),
	m_player(),
	m_level(START_LEVEL),
	m_cameraPosition(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre)
{
	UIMANAGER.Load("ui.xml");

	TransitionLevel(START_LEVEL);
}

Game::~Game() = default;

void Game::Update(const float deltaTime)
{
	m_player.Update(deltaTime);

	m_cameraPosition = maths::SmoothDamp(m_cameraPosition, m_player.GetPosition(), m_cameraVelocity, 0.25, deltaTime);

	m_renderer.SetCameraCentre(m_cameraPosition, m_level.GetNumColumns(), m_level.GetNumRows());

	// If the player is on a door, start a level transition
	if (const DoorData* doorData = m_level.GetDoorPlayerIsOver(m_player.GetGridPosition()))
	{
		TransitionLevel(doorData->m_LevelToLoad);
	}
}

void Game::Render(sf::RenderWindow& window) const
{
	m_renderer.Render(window, m_player);

#if BUILD_DEBUG && 0
	sf::RectangleShape player({ 32, 32 });
	player.setFillColor({ 0, 0, 255, 128 });
	player.setPosition(m_player.GetPosition());
	window.draw(player);

	sf::CircleShape cameraReticle(10);
	player.setOrigin({ cameraReticle.getLocalBounds().size.x / 2.f, cameraReticle.getLocalBounds().size.y / 2.f });
	cameraReticle.setFillColor({ 255, 255, 255, 128 });
	cameraReticle.setPosition(m_cameraPosition);
	window.draw(cameraReticle);
#endif


	window.setView({
		static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre),
		static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize)
		});

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

void Game::TransitionLevel(const std::filesystem::path& newLevel)
{
	m_level = Level(newLevel);
	m_player.SetLevel(&m_level);

	m_player.SetPosition(3 * 32, 3 * 32);
	m_cameraPosition = m_player.GetPosition();

	m_renderer.BuildBatches(m_level.GetRenderData(), m_level.GetLayers());
}
