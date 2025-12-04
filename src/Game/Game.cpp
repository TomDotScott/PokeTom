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
	m_adjacentLevelLoaded(false),
	m_numVisibleCols(m_world.GetLevel(m_world.GetCurrentLevelName())->GetNumColumns()),
	m_numVisibleRows(m_world.GetLevel(m_world.GetCurrentLevelName())->GetNumRows())
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

void Game::ChangeChunk(const std::shared_ptr<Level>& currentLevel, const std::string& adjacentLevelName, const eOrientation orientation, const sf::Vector2i& playerGridPosition)
{
	if (adjacentLevelName.empty())
	{
		return;
	}

	const auto& adjacentLevel = m_world.GetLevel(adjacentLevelName);

	if (adjacentLevel == nullptr)
	{
		std::cerr << "Game::ChangeChunk - No level with name " << adjacentLevelName << " exists!\n";
		return;
	}

	if (m_adjacentLevelLoaded)
	{
		if ((orientation == eOrientation::Up && playerGridPosition.y <= -1) ||
			(orientation == eOrientation::Down && playerGridPosition.y >= static_cast<int>(adjacentLevel->GetNumRows() - 1)) || 
			(orientation == eOrientation::Left && playerGridPosition.x <= -1) || 
			(orientation == eOrientation::Right && playerGridPosition.x >= static_cast<int>(adjacentLevel->GetNumColumns() - 1)))
		{
			m_world.SetCurrentLevel(adjacentLevelName);
		}
		return;
	}

	sf::Vector2f adjacentLevelOffset(0.f, 0.f);
	if (orientation == eOrientation::Up)
	{
		adjacentLevelOffset = static_cast<sf::Vector2f>(adjacentLevel->GetOffsetFromOrigin());
	}
	else if (orientation == eOrientation::Down)
	{
		adjacentLevelOffset = m_adjacentMapOffset;
	}

	BuildAdjacentLevelRenderData(currentLevel, adjacentLevel, adjacentLevelOffset);
}

void Game::BuildAdjacentLevelRenderData(const std::shared_ptr<Level>& currentLevel, const std::shared_ptr<Level>& adjacentLevel, const sf::Vector2f& newOffset)
{
	const auto adjacentLevelRenderData = adjacentLevel->GetRenderData();
	const auto currentLevelRenderData = currentLevel->GetRenderData();

	std::vector<TileRenderData> renderData;
	renderData.insert(renderData.end(), currentLevelRenderData.begin(), currentLevelRenderData.end());
	renderData.insert(renderData.end(), adjacentLevelRenderData.begin(), adjacentLevelRenderData.end());

	const auto adjacentLevelLayerData = adjacentLevel->GetLayers();
	const auto currentLevelLayerData = currentLevel->GetLayers();

	std::vector<TileLayerData> layerData;
	layerData.insert(layerData.end(), currentLevelLayerData.begin(), currentLevelLayerData.end());
	layerData.insert(layerData.end(), adjacentLevelLayerData.begin(), adjacentLevelLayerData.end());

	m_renderer.BuildBatches(renderData, layerData);

	m_numVisibleRows = adjacentLevel->GetNumRows() + currentLevel->GetNumRows();
	m_adjacentMapOffset = newOffset;

	std::cout << "LOADED AND RENDERING ADJACENT LEVEL " << adjacentLevel->GetName() << "\n";

	m_adjacentLevelLoaded = true;
}

void Game::UpdateContinuousWorld()
{
	// TODO: This is terrible, think about this properly!
	const std::string& currentLevelName = m_world.GetCurrentLevelName();
	const std::shared_ptr<Level>& currentLevel = m_world.GetLevel(currentLevelName);
	const Level::AdjacentLevels& adjacentLevels = currentLevel->GetAdjacentLevels();
	const sf::Vector2i playerGridPosition = currentLevel->GetGridPositionFromWorldPosition(m_player.GetPosition());

	if (playerGridPosition.y < static_cast<int>(currentLevel->GetNumRows() / 3))
	{
		ChangeChunk(currentLevel, adjacentLevels.m_North, eOrientation::Up, playerGridPosition);
	}
	else if (playerGridPosition.y > static_cast<int>(currentLevel->GetNumRows() / 3) * 2)
	{
		ChangeChunk(currentLevel, adjacentLevels.m_South, eOrientation::Down, playerGridPosition);
	}
	else
	{
		if (m_adjacentLevelLoaded) {
			LoadLevel(m_world.GetCurrentLevelName(), "", false);
		}

		m_adjacentLevelLoaded = false;
	}
}

void Game::UpdateOverworld(const float deltaTime)
{
	m_player.Update(deltaTime, m_world);
	UpdateContinuousWorld();
	UpdateCamera(deltaTime);
	CheckForPortals();
}

void Game::UpdateCamera(const float deltaTime)
{
	m_cameraPosition = maths::SmoothDamp(m_cameraPosition, m_player.GetPosition(), m_cameraVelocity, 0.25, deltaTime);

	m_renderer.SetCameraCentre(m_cameraPosition,
		m_numVisibleCols,
		m_numVisibleRows,
		m_adjacentMapOffset
	);
}

void Game::CheckForPortals()
{
	const std::shared_ptr<Level> currentLevel = m_world.GetLevel(m_world.GetCurrentLevelName());

	// If the player is on a door in the right orientation, start a level transition
	const PortalTrigger* portal = currentLevel->GetPortalAtPosition(m_player.GetPosition());

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

void Game::LoadLevel(const std::string& levelName, const std::string& spawnPointName, const bool shouldSetPlayerPosition)
{
	const std::shared_ptr<Level> level = m_world.GetLevel(levelName);

	m_numVisibleRows = level->GetNumRows();
	m_numVisibleCols = level->GetNumColumns();
	m_adjacentMapOffset = static_cast<sf::Vector2f>(level->GetOffsetFromOrigin());

	if (shouldSetPlayerPosition)
	{
		const SpawnPointData& spawnPointData = level->GetSpawnPointData(spawnPointName);

		m_player.SetPosition(static_cast<sf::Vector2f>(spawnPointData.m_GridPosition * 32));
		m_player.SetOrientation(spawnPointData.m_Orientation);

		m_cameraPosition = m_player.GetPosition();
		m_renderer.SetCameraCentre(
			m_cameraPosition,
			level->GetNumColumns(),
			level->GetNumRows(),
			m_adjacentMapOffset
		);
	}

	m_renderer.BuildBatches(level->GetRenderData(), level->GetLayers());
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
		LoadLevel(m_world.GetCurrentLevelName(), "player_spawn", true);
		return;
	}

	if (auto transition = m_world.EnterPortal(m_lastEnteredPortal->m_Name))
	{
		LoadLevel(transition->m_NewLevelName, transition->m_SpawnPointName, true);
	}
	else
	{
		std::cerr << "Game::OnTransitionEnd - Failed to transition to portal!\n";
	}

	m_adjacentLevelLoaded = false;
	m_lastEnteredPortal = nullptr;
}
