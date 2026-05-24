#include "Game.h"
#include <fstream>
#include <iostream>
#include <set>
#include <SFML/Graphics.hpp>

#include "BattleState.h"
#include "GameEvents.h"
#include "OverworldState.h"
#include "../Engine/Animation/AnimationComponent.h"
#include "../Engine/Asserts.h"
#include "../Engine/EntityRegistry.h"
#include "../Engine/Stringtable.h"
#include "../Engine/Globals.h"
#include "../Engine/GridMovementComponent.h"
#include "../Engine/Maths.h"
#include "../Engine/Timer.h"

Game::Game(sol::state& lua) :
	IUpdateable(),
	m_context({
		.m_Entities = EntityRegistry(),
		.m_World = WorldDefinition(lua, "WorldDefinition.xml"),
		.m_Renderer = Renderer(),
		.m_PlayerEntityID = 0x69
	}),
	m_luaBindings(lua, m_context.m_World, m_context.m_Entities)
{
	// TODO: Make this a variable, loaded at startup
	StringTable::Get()->AddCustomString(HASH("CHARACTER"), HASH("PLAYER_NAME"), "Tom");
	StringTable::Get()->AddCustomString(HASH("CHARACTER"), HASH("RIVAL_NAME"), "Ben");

	m_context.m_PlayerEntityID = m_context.m_Entities.Create<Player>().GetID();

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

	m_currentState = std::make_unique<OverworldState>(m_context, HASH(START_LEVEL), std::nullopt);


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
		RequestTransition(std::make_unique<OverworldState>(m_context, ctx.m_LevelHash, ctx.m_PlayerPosition));
	});
}

Game::~Game() = default;

void Game::Update(const float deltaTime)
{
	if (m_screenFader.FadeInProgress())
	{
		UpdateScreenFade(deltaTime);
	}
	else
	{
		m_currentState->Update(deltaTime);
	}
}

void Game::Render(sf::RenderWindow& window) const
{
	window.setView({
		static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre),
		static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize)
	});

	m_currentState->Render(window);

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
	m_currentState->OnExit();

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
				m_currentState = std::move(m_pendingState);
				m_currentState->OnEnter();
			}

			m_screenFader.StartFade(ScreenFader::FadeType::FadeIn, 1.f, 0.5f);
			game_events::OnScreenFaded.Fire();
		}
	}
}
