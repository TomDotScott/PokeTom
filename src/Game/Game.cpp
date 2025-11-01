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


Game::Game() :
	Updateable(),
	m_player(),
	m_level("tiled_export\\starter_town.tmj"),
	m_cameraPosition(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre)
{
	UIMANAGER.Load("ui.xml");

	m_player.SetLevel(&m_level);

	m_player.SetPosition(35.f * 32.f, 17.f * 32.f);
	m_cameraPosition = m_player.GetPosition();

	m_renderer.BuildBatches(m_level.GetRenderData(), m_level.GetLayers());
}

Game::~Game() = default;

void Game::Update(const float deltaTime)
{
	m_player.Update(deltaTime);

	m_cameraPosition = maths::SmoothDamp(m_cameraPosition, m_player.GetPosition(), m_cameraVelocity, 0.25, deltaTime);

	m_renderer.SetCameraCentre(m_cameraPosition, m_level.GetNumColumns(), m_level.GetNumRows());
}

void Game::Render(sf::RenderWindow& window) const
{
	m_renderer.Render(window);

#if BUILD_DEBUG
	sf::RectangleShape player({ 32, 32 });
	// player.setOrigin({ player.getLocalBounds().size.x, player.getLocalBounds().size.y });
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
