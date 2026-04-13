#include "Game.h"
#include <fstream>
#include <iostream>
#include <set>
#include <SFML/Graphics.hpp>

#include "../Engine/Asserts.h"
#include "../Engine/TextureManager.h"
#include "../Engine/Globals.h"
#include "../Engine/GridMovementComponent.h"
#include "../Engine/Maths.h"
#include "../Engine/Timer.h"
#include "../Engine/Animation/AnimationComponent.h"
#include "../Engine/CodeGen/Resources.hpp"

static constexpr const char* START_LEVEL = "starter_town";

Game::Game(sol::state& lua) :
	IUpdateable(),
	m_state(eGameState::Overworld),
	m_world(lua, "WorldDefinition.xml"),
	m_luaBindings(lua, m_world, m_entities),
	m_lastEnteredPortal(nullptr),
	m_cameraPosition(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre),
	m_worldBounds({ 0, 0 }, { static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize) }),
	m_lastCameraRect({ 0.f, 0.f }, { 0.f, 0.f }),
	m_cameraRebuildThreshold(10.f * 32.f)
{
	m_playerEntityID = m_entities.Create<Player>().GetID();

	Player* player = m_entities.Get<Player>(m_playerEntityID);
	player->SetCanMoveCallback([&](const Entity* ent, const eDirection dir)
	{
		return m_world.CanMoveTo(ent, dir);
	});

	UIMANAGER.Load("ui.xml");

	m_world.LoadLevelScripts(lua);

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

	UpdateChunks();
	UpdateCamera(deltaTime);
}

void Game::Render(sf::RenderWindow& window) const
{
	const Player* player = m_entities.Get<Player>(m_playerEntityID);

	m_renderer.Render(window, m_entities, m_world.GetLevelAtPosition(player->GetPosition())->GetEntityZIndex());

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

	if (m_screenFader.FadeInProgress())
	{
		const uint8_t screenAlpha = m_screenFader.GetScreenAlpha();

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

void Game::UpdateChunks()
{
	const sf::Vector2f viewSize = m_worldBounds.size;
	const sf::FloatRect camRect(
		{
			m_cameraPosition.x - viewSize.x * 0.5f,
			m_cameraPosition.y - viewSize.y * 0.5f
		},
		viewSize
	);

	if (m_lastCameraRect.size.lengthSquared() == 0.f ||
		(camRect.position - m_lastCameraRect.position).lengthSquared() >
		m_cameraRebuildThreshold * m_cameraRebuildThreshold)
	{
		const auto visibleLevels = m_world.GetLevelsIntersectingRect(camRect);

		std::unordered_map<std::string, LevelRenderData> renderData;

		sf::FloatRect mergedBounds;
		bool first = true;

		for (const auto& level : visibleLevels)
		{
			const auto& levelName = level->GetName();
			renderData[levelName] = LevelRenderData();

			auto& levelRenderData = renderData.at(levelName);

			levelRenderData.m_TileRenderData = level->GetRenderData();
			levelRenderData.m_TileLayerData = level->GetLayers();

			sf::FloatRect r = level->GetBounds();

			if (first)
			{
				mergedBounds = r;
				first = false;
			}
			else
			{
				// Union the rectangles
				float minX = std::min(mergedBounds.position.x, r.position.x);
				float minY = std::min(mergedBounds.position.y, r.position.y);

				float maxX = std::max(mergedBounds.position.x + mergedBounds.size.x,
					r.position.x + r.size.x);
				float maxY = std::max(mergedBounds.position.y + mergedBounds.size.y,
					r.position.y + r.size.y);

				mergedBounds = sf::FloatRect(
					sf::Vector2f(minX, minY),
					sf::Vector2f(maxX - minX, maxY - minY)
				);
			}
		}

		// Update renderer data
		m_renderer.BuildBatches(renderData);

		// Update world boundary the camera clamps inside
		m_worldBounds = mergedBounds;

		m_lastCameraRect = camRect;
	}
}


void Game::UpdateOverworld(const float deltaTime)
{
	m_world.GetLevelAtPosition(m_entities.Get<Entity>(m_playerEntityID)->GetPosition())->OnUpdate(deltaTime);
	m_entities.UpdateAll(deltaTime);
	CheckForPortals();
}

void Game::UpdateCamera(const float deltaTime)
{
	Player* player = m_entities.Get<Player>(m_playerEntityID);

	m_cameraPosition = maths::SmoothDamp(m_cameraPosition, player->GetPosition(), m_cameraVelocity, 0.25, deltaTime);

	m_renderer.SetCameraCentre(m_cameraPosition, m_worldBounds);
}

void Game::CheckForPortals()
{
	Player* player = m_entities.Get<Player>(m_playerEntityID);

	const auto currentLevel = m_world.GetLevelAtPosition(player->GetPosition());

	if (currentLevel == nullptr)
	{
		std::cerr << "Game::CheckForPortals - currentLevel is a nullptr!\n";
		return;
	}

	// If the player is on a door in the right orientation, start a level transition
	const PortalTrigger* portal = currentLevel->GetPortalAtPosition(player->GetPosition());

	GridMovementComponent* playerMovement = player->GetComponent<GridMovementComponent>();
	if (portal != nullptr && portal->AllowsOrientation(playerMovement->GetCurrentOrientation()))
	{
		m_lastEnteredPortal = portal;

		std::cout << "Transitioning to " << m_world.GetPortalData(currentLevel->GetName(), portal->m_Name).m_TargetLevel
			<< "\n";

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
			OnTransitionEnd();
			m_screenFader.StartFade(ScreenFader::FadeType::FadeIn, 1.f, 0.5f);
		}
		else if (m_screenFader.GetCurrentFadeType() == ScreenFader::FadeType::FadeIn)
		{
			m_state = eGameState::Overworld;
		}
	}
}

void Game::RespawnPlayer(const std::string& levelName, const std::string& spawnPointName,
	const bool shouldSetPlayerPosition)
{
	const std::shared_ptr<Level> level = m_world.GetLevel(levelName);

	ASSERT(level != nullptr);

	if (shouldSetPlayerPosition)
	{
		const SpawnPointData& spawnPointData = level->GetSpawnPointData(spawnPointName);

		Player* player = m_entities.Get<Player>(m_playerEntityID);

		GridMovementComponent* playerMovement = player->GetComponent<GridMovementComponent>();
		playerMovement->StopMoving();
		playerMovement->SetDirection(spawnPointData.m_Orientation);
		player->SetPosition(static_cast<sf::Vector2f>(level->GetWorldOrigin() + spawnPointData.m_GridPosition * 32));

		player->GetComponent<EntityAnimationComponent>()->PlayAnimation(
			EntityAnimationComponent::GetIdleAnimation(spawnPointData.m_Orientation), true);

		m_cameraPosition = player->GetPosition();

		m_worldBounds = level->GetBounds();
	}
}

void Game::TransitionLevel()
{
	m_state = eGameState::FadingScreen;
	m_screenFader.StartFade(ScreenFader::FadeType::FadeOut, 1.f, 0.5f);
}

void Game::OnTransitionEnd()
{
	Player* player = m_entities.Get<Player>(m_playerEntityID);

	if (m_lastEnteredPortal == nullptr)
	{
		RespawnPlayer(START_LEVEL, "player_spawn", true);
		m_world.GetLevelAtPosition(player->GetPosition())->OnActivate();
		return;
	}


	if (auto transition = m_world.EnterPortal(m_world.GetLevelAtPosition(player->GetPosition())->GetName(),
		m_lastEnteredPortal->m_Name))
	{
		RespawnPlayer(transition->m_NewLevelName, transition->m_SpawnPointName, true);
	}
	else
	{
		std::cerr << "Game::OnTransitionEnd - Failed to transition to portal!\n";
	}

	m_lastEnteredPortal = nullptr;
}
