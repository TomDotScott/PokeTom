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
	m_state(eGameState::Overworld),
	m_player(),
	m_world("WorldDefinition.xml", "player_bedroom"),
	m_lastEnteredPortal(nullptr),
	m_cameraPosition(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre)
{
	UIMANAGER.Load("ui.xml");

	ReadyPlayerAndRenderer();
}

Game::~Game() = default;

void Game::Update(const float deltaTime)
{
	switch (m_state)
	{
	case eGameState::FadingScreen:
		UpdateScreenFade(deltaTime);
		break;
	case eGameState::Overworld:
		UpdateOverworld(deltaTime);
		break;
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

	const uint8_t screenAlpha = m_screenFader.GetScreenAlpha();
	if (screenAlpha != 255)
	{
		sf::RectangleShape screenFade{ static_cast<sf::Vector2f>(window.getSize()) };
		screenFade.setFillColor({ 0, 0, 0, screenAlpha });
		window.draw(screenFade);
	}

	/* TODO: Integrate UI into the Renderer
	// Draw the background items first
	UIMANAGER.RenderBackground(window);

	// Then the midground stuff
	UIMANAGER.RenderMidground(window);

	// Then, in front of everything, the foreground stuff
	UIMANAGER.RenderForeground(window);
	*/

#if !BUILD_MASTER
	DrawText(window, sf::Vector2f{ 0, 10 }, 30, "%.1fFPS", Timer::Get().Fps());
#endif
}

void Game::UpdateOverworld(const float deltaTime)
{
	m_player.Update(deltaTime);

	m_cameraPosition = maths::SmoothDamp(m_cameraPosition, m_player.GetPosition(), m_cameraVelocity, 0.25, deltaTime);

	const auto currentLevel = m_world.GetLevel(m_world.GetCurrentLevelName());

	m_renderer.SetCameraCentre(m_cameraPosition, currentLevel->GetNumColumns(), currentLevel->GetNumRows());

	// If the player is on a door in the right orientation, start a level transition
	const PortalData* portalData = currentLevel->GetDoorPlayerIsOver(m_player.GetGridPosition());

	if (portalData != nullptr)
	{
		const uint8_t playerOrientation = static_cast<uint8_t>(m_player.GetCurrentOrientation());

		if (portalData->m_Orientation & playerOrientation) {
			m_lastEnteredPortal = portalData;

			std::cout << "Transitioning to " << m_world.GetPortalData(m_world.GetCurrentLevelName(), portalData->m_Name).m_TargetLevel << "\n";

			TransitionLevel();
		}
	}
}

void Game::UpdateScreenFade(const float deltaTime)
{
	m_screenFader.Update(deltaTime);

	if (!m_screenFader.FadeInProgress())
	{
		// If we have finished the fade out, fade back in again
		if (m_screenFader.GetCurrentFadeType() == ScreenFader::FadeType::FadeOut)
		{
			m_screenFader.StartFade(ScreenFader::FadeType::FadeIn, 1.f, 0.5f);
			ReadyPlayerAndRenderer();
		}
		else if (m_screenFader.GetCurrentFadeType() == ScreenFader::FadeType::FadeIn)
		{
			m_state = eGameState::Overworld;
		}
	}
}

void Game::TransitionLevel()
{
	m_state = eGameState::FadingScreen;
	m_screenFader.StartFade(ScreenFader::FadeType::FadeOut, 1.f, 0.5f);
}

void Game::ReadyPlayerAndRenderer()
{
	std::string levelName = m_world.GetCurrentLevelName();
	std::shared_ptr<Level> level = m_world.GetLevel(levelName);

	// TODO: Remove this little hack!
	if (m_lastEnteredPortal == nullptr)
	{
		m_player.SetGridPosition({ 3, 3 });
		m_player.SetOrientation(eOrientation::Down);
	}
	else
	{
		const auto& portalData = m_world.GetPortalData(levelName, m_lastEnteredPortal->m_Name);
		m_world.OnPlayerEnterPortal(m_lastEnteredPortal->m_Name);

		level = m_world.GetLevel(m_world.GetCurrentLevelName());

		const SpawnPointData& spawnPointData = level->GetSpawnPointData(portalData.m_TargetSpawnPoint);

		m_player.SetGridPosition(spawnPointData.m_GridPosition);
		m_player.SetOrientation(spawnPointData.m_Orientation);
	}

	m_player.SetLevel(level);

	m_renderer.BuildBatches(level->GetRenderData(), level->GetLayers());

	m_cameraPosition = m_player.GetPosition();
	m_renderer.SetCameraCentre(m_cameraPosition, level->GetNumColumns(), level->GetNumRows());

	m_lastEnteredPortal = nullptr;
}
