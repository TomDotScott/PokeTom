#include "Game.h"
#include <fstream>
#include <iostream>
#include <set>
#include <SFML/Graphics.hpp>
#include "../Engine/TextureManager.h"
#include "../Engine/Globals.h"
#include "../Engine/Maths.h"
#include "../Engine/Timer.h"
#include "../Engine/Input/Keyboard.h"


Game::Game() :
	m_playerPosition(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre),
	m_cameraPosition(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre),
	m_mapData(TileParser::ParseTMJ("tiled_export\\starter_town.tmj")),
	m_tileLogic(m_mapData)
{
	UIMANAGER.Load("ui.xml");

	m_mapper.Map(UP, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::W));
	m_mapper.Map(DOWN, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::S));

	m_mapper.Map(LEFT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::A));
	m_mapper.Map(RIGHT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::D));

	auto renderData = m_tileLogic.BuildRenderData();
	m_renderer.BuildBatches(renderData, m_mapData.m_Layers);
}

Game::~Game() = default;

void Game::Update()
{
	m_mapper.Update();

	constexpr float speed = 32.f * 5;

	const float deltaTime = Timer::Get().DeltaTime();

	if (m_mapper.IsButtonDown(UP))
	{
		m_playerPosition.y -= speed * deltaTime;
	}
	else if (m_mapper.IsButtonDown(DOWN))
	{
		m_playerPosition.y += speed * deltaTime;
	}

	if (m_mapper.IsButtonDown(LEFT))
	{
		m_playerPosition.x -= speed * deltaTime;
	}
	else if (m_mapper.IsButtonDown(RIGHT))
	{
		m_playerPosition.x += speed * deltaTime;
	}

	m_cameraPosition = maths::SmoothDamp(m_cameraPosition, m_playerPosition, m_cameraVelocity, 0.25, deltaTime);

	m_renderer.SetCameraCentre(m_cameraPosition, m_mapData.m_NumColumns, m_mapData.m_NumRows);
}

void Game::Render(sf::RenderWindow& window) const
{
	m_renderer.Render(window);

#if BUILD_DEBUG
	sf::RectangleShape player({ 20, 20 });
	// player.setOrigin({ player.getLocalBounds().size.x, player.getLocalBounds().size.y });
	player.setFillColor({ 0, 0, 255, 128 });
	player.setPosition(m_playerPosition);
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
