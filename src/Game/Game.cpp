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
	m_cameraPosition(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre),
	m_adjacentLevelLoaded(false)
{
	UIMANAGER.Load("ui.xml");

	OnTransitionEnd();
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

	const sf::Vector2i playerGridPosition = m_player.GetGridPosition();
	const std::string& currentLevelName = m_world.GetCurrentLevelName();
	const std::shared_ptr<Level>& currentLevel = m_world.GetLevel(currentLevelName);
	const Level::AdjacentLevels& adjacentLevels = currentLevel->GetAdjacentLevels();

	if (!m_adjacentLevelLoaded) {
		if (playerGridPosition.y < currentLevel->GetNumColumns() - 8)
		{
			if (!adjacentLevels.m_North.empty())
			{
				const auto& northLevel = m_world.GetLevel(adjacentLevels.m_North);
				const auto northLevelRenderData = northLevel->GetRenderData();
				const auto currentLevelRenderData = currentLevel->GetRenderData();

				std::vector<TileRenderData> renderData;
				renderData.insert(renderData.end(), currentLevelRenderData.begin(), currentLevelRenderData.end());
				renderData.insert(renderData.end(), northLevelRenderData.begin(), northLevelRenderData.end());

				const auto northLevelLayerData = northLevel->GetLayers();
				const auto currentLevelLayerData = currentLevel->GetLayers();

				std::vector<TileLayerData> layerData;
				layerData.insert(layerData.end(), currentLevelLayerData.begin(), currentLevelLayerData.end());
				layerData.insert(layerData.end(), northLevelLayerData.begin(), northLevelLayerData.end());

				m_renderer.BuildBatches(renderData, layerData);

				std::cout << "LOADED AND RENDERING ADJACENT LEVEL " << adjacentLevels.m_North << "\n";

				m_adjacentLevelLoaded = true;
			}
		}
		else
		{
			m_adjacentLevelLoaded = false;
		}
	}

	UpdateCamera(deltaTime, 100, 100);
	CheckForPortals();
}

void Game::UpdateCamera(const float deltaTime, const uint32_t numCols, const uint32_t numRows)
{
	m_cameraPosition = maths::SmoothDamp(m_cameraPosition, m_player.GetPosition(), m_cameraVelocity, 0.25, deltaTime);

	m_renderer.SetCameraCentre(m_cameraPosition,
		numCols,
		numRows
	);
}

void Game::CheckForPortals()
{
	const std::shared_ptr<Level> currentLevel = m_world.GetLevel(m_world.GetCurrentLevelName());

	// If the player is on a door in the right orientation, start a level transition
	const PortalTrigger* portal = currentLevel->GetPortalAtPlayerPosition(m_player.GetGridPosition());

	if (portal != nullptr && portal->AllowsOrientation(m_player.GetCurrentOrientation()))
	{
		m_lastEnteredPortal = portal;

		std::cout << "Transitioning to " << m_world.GetPortalData(m_world.GetCurrentLevelName(), portal->m_Name).m_TargetLevel << "\n";

		TransitionLevel();
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
			OnTransitionEnd();
		}
		else if (m_screenFader.GetCurrentFadeType() == ScreenFader::FadeType::FadeIn)
		{
			m_state = eGameState::Overworld;
		}
	}
}

void Game::LoadLevel(const std::string& levelName, const std::string& spawnPointName)
{
	const std::shared_ptr<Level> level = m_world.GetLevel(levelName);

	const SpawnPointData& spawnPointData = level->GetSpawnPointData(spawnPointName);

	m_player.SetGridPosition(spawnPointData.m_GridPosition);
	m_player.SetOrientation(spawnPointData.m_Orientation);

	m_player.SetLevel(level);

	m_renderer.BuildBatches(level->GetRenderData(), level->GetLayers());

	m_cameraPosition = m_player.GetPosition();
	m_renderer.SetCameraCentre(m_cameraPosition, level->GetNumColumns(), level->GetNumRows());
}

void Game::TransitionLevel()
{
	m_state = eGameState::FadingScreen;
	m_screenFader.StartFade(ScreenFader::FadeType::FadeOut, 1.f, 0.5f);
}

void Game::OnTransitionEnd()
{
	if (m_lastEnteredPortal == nullptr)
	{
		LoadLevel(m_world.GetCurrentLevelName(), "player_spawn");
		return;
	}

	if (auto transition = m_world.EnterPortal(m_lastEnteredPortal->m_Name))
	{
		LoadLevel(transition->m_NewLevelName, transition->m_SpawnPointName);
	}
	else
	{
		std::cerr << "Game::OnTransitionEnd - Failed to transition to portal!\n";
	}

	m_lastEnteredPortal = nullptr;
}
