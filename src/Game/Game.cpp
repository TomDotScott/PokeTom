#include "Game.h"
#include <fstream>
#include <set>
#include <SFML/Graphics.hpp>

#include "BattleState.h"
#include "GameEvents.h"
#include "OverworldState.h"
#include "../Engine/Asserts.h"
#include "../Engine/EntityRegistry.h"
#include "../Engine/Globals.h"
#include "../Engine/GridMovementComponent.h"
#include "../Engine/Maths.h"
#include "../Engine/Stringtable.h"
#include "../Engine/Timer.h"
#include "Monsters/MonsterPartyComponent.h"
#include "Monsters/PocketMonsterManager.h"

Game::Game(sol::state& lua) :
	IUpdateable(),
	m_context({
		.m_Entities = EntityRegistry(),
		.m_World = WorldDefinition(lua, "WorldDefinition.xml"),
		.m_Renderer = Renderer(),
		.m_PlayerEntityID = 0x69
	}),
	m_luaBindings(lua, m_context.m_World, m_context.m_Entities),
	m_lastHospitalCheckpoint({
		.m_LevelHash = HASH(START_LEVEL),
		.m_SpawnPointHash = HASH("player_spawn")
	})
{
	// TODO: Make this a variable, loaded at startup
	StringTable::Get()->AddCustomString(HASH("CHARACTER"), HASH("PLAYER_NAME"), "Tom");
	StringTable::Get()->AddCustomString(HASH("CHARACTER"), HASH("RIVAL_NAME"), "Ben");

	m_context.m_PlayerEntityID = m_context.m_Entities.Create<Player>(sf::Vector2f(0.f, 0.f)).GetID();

	Player* player = m_context.m_Entities.Get<Player>(m_context.m_PlayerEntityID);
	player->SetCanMoveCallback([&](const Entity* ent, const eDirection dir)
	{
		return m_context.m_World.CanMoveTo(ent, m_context.m_Entities, dir);
	});

	player->AddOnInteractPressedCallback([this]()
	{
		// See if there are any entities near the player
		Player* p = m_context.m_Entities.Get<Player>(m_context.m_PlayerEntityID);

		const GridMovementComponent* playerMovement = p->GetComponent<GridMovementComponent>();
		const sf::Vector2f nextPlayerPos = playerMovement->GetNextPosition(playerMovement->GetCurrentDirection());

		const entity_id_t entityID = m_context.m_Entities.GetEntityAtPosition(nextPlayerPos);
		if (entityID == ~0U)
		{
			// TODO: Play a sound or something here
			return;
		}

		const auto& entity = m_context.m_Entities.Get<Entity>(entityID);
		entity->OnPlayerInteractPressed();
	});

	ASSERT_MSG(UIMANAGER.Load("ui.xml"), "UI Failed to initialise from XML!");

	m_context.m_World.LoadLevelScripts(lua);

	game_events::OnScreenFadeTriggered.On([this]()
	{
		m_screenFader.StartFade(ScreenFader::FadeType::FadeOut, 1.f, 0.5f);
	});

	game_events::OnBattleStart.On([this](const BattleBeginContext& battleContext)
	{
		RequestTransition(std::make_unique<BattleState>(m_context, battleContext));
	});

	game_events::OnBattleEnd.On([this](const BattleEndContext& ctx)
	{
		if (ctx.m_SendPlayerToHospital)
		{
			// TODO: Proper transition to some dialogue saying that the player whited out and rushed back to the hospital
			const std::shared_ptr<Level> level = m_context.m_World.GetLevel(m_lastHospitalCheckpoint.m_LevelHash);
			sf::Vector2f spawnPoint = level->GetWorldPositionFromGridPosition(
				level->GetSpawnPointData(m_lastHospitalCheckpoint.m_SpawnPointHash).m_GridPosition);
			RequestTransition(
				std::make_unique<OverworldState>(m_context, m_lastHospitalCheckpoint.m_LevelHash, spawnPoint, true));
		}
		else
		{
			RequestTransition(
				std::make_unique<OverworldState>(m_context, ctx.m_LevelHash, ctx.m_PlayerPosition, false));
		}
	});

	RequestTransition(std::make_unique<OverworldState>(m_context, HASH(START_LEVEL), std::nullopt, false));

#if !BUILD_MASTER
	m_mapper.Map(TRIGGER_BATTLE, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::F5));

	m_mapper.OnButtonPressed(TRIGGER_BATTLE, [this]()
	{
		static Entity* debugOpponent = nullptr;

		if (debugOpponent != nullptr)
		{
			m_context.m_Entities.Destroy(debugOpponent->GetID());
			debugOpponent = nullptr;
		}

		auto player = m_context.m_Entities.Get<Player>(m_context.m_PlayerEntityID);
		debugOpponent = &m_context.m_Entities.Create();

		auto& mpc = debugOpponent->AddComponent<MonsterPartyComponent>(
			debugOpponent,
			m_context,
			std::vector<MonsterPartyComponent::MonsterPartyInfo>{
				{
					.m_Monster = PocketMonsterManager::Get()->GetMonsterDetails(403),
					.m_Level = 12
				},
			}
		);

		mpc.OnActivate();

		game_events::OnBattleStart.Fire({
			.m_LevelHash = m_context.m_World.GetLevelAtPosition(player->GetPosition())->GetName(),
			.m_PlayerPosition = player->GetPosition(),
			.m_PlayerEntityID = player->GetID(),
			.m_OpponentEntityID = debugOpponent->GetID(),
			.m_isTrainerBattle = false,
		});

		game_events::OnBattleEnd.Once([this](const BattleEndContext& /*ctx*/)
		{
			printf("DEBUG BATTLE ENDED!\n");

			game_events::OnScreenFaded.Once([this]()
			{
				m_context.m_Entities.Destroy(debugOpponent->GetID());
				debugOpponent = nullptr;
			});
		});
	});
#endif
}

Game::~Game() = default;

void Game::Update(const float deltaTime)
{
#if !BUILD_MASTER
	m_mapper.Update();
#endif

	if (m_screenFader.FadeInProgress())
	{
		UpdateScreenFade(deltaTime);
	}
	else
	{
		if (m_currentState != nullptr)
		{
			m_currentState->Update(deltaTime);
		}
	}

	m_context.m_Renderer.UpdateAnimatedTiles(deltaTime);
}

void Game::Render(sf::RenderWindow& window) const
{
	window.setView({
		static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre),
		static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize)
	});

	if (m_currentState != nullptr)
	{
		m_currentState->Render(window);
	}

	if (m_screenFader.FadeInProgress())
	{
		const uint8_t screenAlpha = m_screenFader.GetScreenAlpha();

		sf::RectangleShape screenFade{ static_cast<sf::Vector2f>(window.getSize()) };
		screenFade.setFillColor({ 0, 0, 0, screenAlpha });
		window.draw(screenFade);
	}

#if !BUILD_MASTER
	DrawText(window, sf::Vector2f{ 0, 10 }, 30, "%.1fFPS", Timer::Get().Fps());
#endif
}

void Game::RequestTransition(std::unique_ptr<IGameState> next)
{
	m_pendingState = std::move(next);
	game_events::OnScreenFadeTriggered.Fire();
}

void Game::UpdateScreenFade(const float deltaTime)
{
	m_screenFader.Update(deltaTime);

	if (!m_screenFader.FadeInProgress())
	{
		// If we have finished the fade out, fade back in again
		if (m_screenFader.GetCurrentFadeType() == ScreenFader::FadeType::FadeOut)
		{
			if (m_pendingState)
			{
				if (m_currentState)
				{
					m_currentState->OnExit();
				}

				m_currentState.reset();
				m_currentState = std::move(m_pendingState);
				m_currentState->OnEnter();
			}

			m_screenFader.StartFade(ScreenFader::FadeType::FadeIn, 1.f, 0.5f);
			game_events::OnScreenFaded.Fire();
		}
	}
}
