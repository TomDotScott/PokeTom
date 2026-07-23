#include "OverworldState.h"

#include <iostream>

#include "GameEvents.h"
#include "../Engine/Asserts.h"
#include "../Engine/GridMovementComponent.h"
#include "../Engine/Maths.h"
#include "../Engine/Animation/AnimationComponent.h"
#include "../Engine/Animation/AnimationComponent.h"
#include "../Engine/UI/UiManager.h"
#include "Monsters/PocketMonsterEntity.h"
#include "Monsters/PocketMonsterManager.h"


// TODO: HACK - Gonna turn this into a proper component on the Player entity (and any NPC entities that need it!)
std::vector<entity_id_t> PLAYER_PARTY;

OverworldState::OverworldState(
	GameContext& ctx,
	const hash_type overworldLevel,
	const std::optional<sf::Vector2f> playerPosition,
	const bool healPlayerParty
) :
	m_gameContext(ctx),
	m_worldBounds({ 0, 0 }, { static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize) }),
	m_lastCameraRect({ 0.f, 0.f }, { 0.f, 0.f }),
	m_cameraRebuildThreshold(),
	m_lastEnteredPortalID(std::nullopt),
	m_cameraPosition(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre),
	m_rng(0, 100)
{
	if (PLAYER_PARTY.empty())
	{
		auto& shinxEntity = ctx.m_Entities.Create<PocketMonsterEntity>(
			403, 5, EntityAnimationComponent::eAnimationName::BATTLE_BACK);
		shinxEntity.OnDeactivate();
		PLAYER_PARTY.emplace_back(shinxEntity.GetID());
	}

	const std::shared_ptr<Level> startLevel = m_gameContext.m_World.GetLevel(overworldLevel);
	m_cameraRebuildThreshold = 10.f * static_cast<float>(startLevel->GetTileWidth());

	if (playerPosition == std::nullopt)
	{
		if (m_lastEnteredPortalID == std::nullopt)
		{
			auto* player = ctx.m_Entities.Get<Entity>(ctx.m_PlayerEntityID);
			ASSERT(player);

			RespawnPlayerAtPortal(HASH(START_LEVEL), HASH("player_spawn"));
			m_gameContext.m_World.GetLevelAtPosition(player->GetPosition())->OnActivate();
		}
		else
		{
			OnLevelEntered();
		}
	}
	else
	{
		RespawnPlayerInWorld(overworldLevel, playerPosition.value());
	}

	if (healPlayerParty)
	{
		const auto& entities = ctx.m_Entities;
		for (auto entityID : PLAYER_PARTY)
		{
			PocketMonsterEntity* monster = entities.Get<PocketMonsterEntity>(entityID);
			ASSERT(monster);

			monster->FullHeal();
		}
	}

	m_onScreenFadedEventID = game_events::OnScreenFaded.On([this]() { OnLevelEntered(); });
}

void OverworldState::OnEnter()
{
	Entity* player = m_gameContext.m_Entities.Get<Entity>(m_gameContext.m_PlayerEntityID);
	m_cameraPosition = player->GetPosition();
	m_gameContext.m_Renderer.SetCameraCentre(m_cameraPosition, m_worldBounds);
}

void OverworldState::OnExit()
{
	game_events::OnScreenFaded.Off(m_onScreenFadedEventID);

	for (const hash_type& hash : m_activeLevelIDs)
	{
		m_gameContext.m_World.GetLevel(hash)->OnDeactivate();
	}

	m_activeLevelIDs.clear();
}

void OverworldState::Update(const float deltaTime)
{
	Entity* player = m_gameContext.m_Entities.Get<Entity>(m_gameContext.m_PlayerEntityID);
	const sf::Vector2f& playerPosition = player->GetPosition();

	const std::shared_ptr<Level> level = m_gameContext.m_World.GetLevelAtPosition(playerPosition);
	level->OnUpdate(deltaTime);

	m_gameContext.m_Entities.UpdateAll(deltaTime);

	CheckForPortals(player, level.get());
	CheckForTallGrass(player, level.get());

	UpdateChunks();
	UpdateCamera(deltaTime);
}

void OverworldState::UpdateCamera(const float deltaTime)
{
	Entity* player = m_gameContext.m_Entities.Get<Entity>(m_gameContext.m_PlayerEntityID);
	m_cameraPosition = maths::SmoothDamp(m_cameraPosition, player->GetPosition(), m_cameraVelocity, 0.25, deltaTime);

	m_gameContext.m_Renderer.SetCameraCentre(m_cameraPosition, m_worldBounds);
}

void OverworldState::UpdateChunks()
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
		const auto visibleLevels = m_gameContext.m_World.GetLevelsIntersectingRect(camRect);

		// TODO: This all really needs to be multi-threaded - it eats up so much CPU time building the render data and executing the Lua
		// Deactivate any levels that are no longer visible
		std::unordered_set<hash_type> newActiveLevels;
		for (const auto& level : visibleLevels)
		{
			newActiveLevels.insert(level->GetName());
		}

		for (const auto& hash : m_activeLevelIDs)
		{
			if (!newActiveLevels.contains(hash))
			{
				m_gameContext.m_World.GetLevel(hash)->OnDeactivate();
			}
		}

		for (const auto& level : visibleLevels)
		{
			if (!m_activeLevelIDs.contains(level->GetName()))
			{
				level->OnActivate();
			}
		}

		m_activeLevelIDs = newActiveLevels;

		std::unordered_map<hash_type, LevelRenderData> renderData;

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
		m_gameContext.m_Renderer.BuildBatches(renderData);

		// Update world boundary the camera clamps inside
		m_worldBounds = mergedBounds;

		m_lastCameraRect = camRect;
	}
}


void OverworldState::Render(sf::RenderWindow& window) const
{
	const Entity* player = m_gameContext.m_Entities.Get<Entity>(m_gameContext.m_PlayerEntityID);
	const std::shared_ptr<Level> level = m_gameContext.m_World.GetLevelAtPosition(player->GetPosition());

	ASSERT(player);
	ASSERT(level);

	m_gameContext.m_Renderer.RenderLevel(window, m_gameContext.m_Entities, level->GetEntityZIndex());

	UIMANAGER.RenderBackground(window);
	UIMANAGER.RenderMidground(window);
	UIMANAGER.RenderForeground(window);
}

void OverworldState::CheckForPortals(Entity* player, const Level* currentLevel)
{
	// If the player is on a door in the right orientation, start a level transition
	const PortalTrigger* portal = currentLevel->GetPortalAtPosition(player->GetPosition());

	if (portal != nullptr)
	{
		GridMovementComponent* playerMovement = player->GetComponent<GridMovementComponent>();

		if (!portal->AllowsOrientation(playerMovement->GetCurrentOrientation()))
		{
			return;
		}

		m_lastEnteredPortalID = portal->m_Name;

		std::cout << "Transitioning to " << m_gameContext.m_World.GetPortalData(currentLevel->GetName(), portal->m_Name).
		                                          m_TargetLevel << "\n";

		game_events::OnScreenFadeTriggered.Fire();
	}
}

void OverworldState::CheckForTallGrass(Entity* player, const Level* currentLevel)
{
	const GridMovementComponent* mvmt = player->GetComponent<GridMovementComponent>();
	if (mvmt == nullptr || !mvmt->JustCompletedMovement())
	{
		return;
	}

	// No wild PocketMonsters to see here, buddy!
	const auto& distribution = currentLevel->GetMonsterDistribution();
	if (distribution == std::nullopt)
	{
		return;
	}

	const sf::Vector2f& playerPos = player->GetPosition();
	if (currentLevel->GetTileAtPosition(playerPos, TileSheet::TileDefinition::IsGrass))
	{
		if (m_rng.Next() >= currentLevel->GetWildPocketMonsterProbability() * 100.f)
		{
			return;
		}

		// Create the entity
		uint32_t monster = distribution.value().ChooseMonsterFromDistribution(MapData::MonsterDistribution::eDistributionType::Grass);

		auto& monsterEntity = m_gameContext.m_Entities.Create<PocketMonsterEntity>(
			monster,
			// TODO: Support Different levels of wild monsters!
			5,
			EntityAnimationComponent::eAnimationName::BATTLE_FRONT
		);

		monsterEntity.OnDeactivate();

		const BattleBeginContext ctx(currentLevel->GetName(),
		                       playerPos,
		                       m_gameContext.m_PlayerEntityID,
		                       ~0U,
		                       false,
		                       PLAYER_PARTY,
		                       { monsterEntity.GetID() }
		);

		game_events::OnBattleStart.Fire(ctx);
	}
}

void OverworldState::HealAllPlayerMonsters()
{

}

void OverworldState::OnLevelEntered()
{
	if (m_lastEnteredPortalID == std::nullopt)
	{
		return;
	}

	Entity* player = m_gameContext.m_Entities.Get<Entity>(m_gameContext.m_PlayerEntityID);

	if (const auto transition = m_gameContext.m_World.EnterPortal(
		m_gameContext.m_World.GetLevelAtPosition(player->GetPosition())->GetName(), m_lastEnteredPortalID.value()))
	{
		RespawnPlayerAtPortal(transition->m_NewLevelName, transition->m_SpawnPointName);
	}
	else
	{
		std::cerr << "Game::OnTransitionEnd - Failed to transition to portal!\n";
	}

	m_lastEnteredPortalID = std::nullopt;
}

void OverworldState::RespawnPlayerAtPortal(const hash_type& levelName, const hash_type& spawnPointName)
{
	const std::shared_ptr<Level> level = m_gameContext.m_World.GetLevel(levelName);
	ASSERT(level != nullptr);

	const SpawnPointData& spawnPointData = level->GetSpawnPointData(spawnPointName);

	Entity* player = m_gameContext.m_Entities.Get<Entity>(m_gameContext.m_PlayerEntityID);

	GridMovementComponent* playerMovement = player->GetComponent<GridMovementComponent>();
	playerMovement->StopMoving();
	playerMovement->SetDirection(spawnPointData.m_Orientation);
	player->SetPosition(static_cast<sf::Vector2f>(level->GetWorldOrigin() + spawnPointData.m_GridPosition *
		static_cast<int>(level->GetTileWidth())));

	player->GetComponent<EntityAnimationComponent>()->PlayAnimation(
		EntityAnimationComponent::GetIdleAnimation(spawnPointData.m_Orientation), true);

	m_cameraPosition = player->GetPosition();

	m_worldBounds = level->GetBounds();


	UpdateChunks();
	UpdateCamera(0.f);
}

void OverworldState::RespawnPlayerInWorld(const hash_type& levelName, const sf::Vector2f& position)
{
	const std::shared_ptr<Level> level = m_gameContext.m_World.GetLevel(levelName);
	ASSERT(level != nullptr);

	Entity* player = m_gameContext.m_Entities.Get<Entity>(m_gameContext.m_PlayerEntityID);

	GridMovementComponent* playerMovement = player->GetComponent<GridMovementComponent>();
	playerMovement->StopMoving();

	player->GetComponent<EntityAnimationComponent>()->PlayAnimation(
		EntityAnimationComponent::GetIdleAnimation(playerMovement->GetCurrentOrientation()), true);

	const sf::Vector2f tileSize{
		static_cast<float>(level->GetTileWidth()),
		static_cast<float>(level->GetTileHeight())
	};

	// Round the position to the nearest grid position
	player->SetPosition(
		static_cast<sf::Vector2f>(static_cast<sf::Vector2i>(position.componentWiseDiv(tileSize))).
		componentWiseMul(tileSize));

	level->OnActivate();

	m_cameraPosition = player->GetPosition();
	m_worldBounds = level->GetBounds();

	UpdateChunks();
	UpdateCamera(0.f);
}
