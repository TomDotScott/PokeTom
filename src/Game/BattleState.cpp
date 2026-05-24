#include "BattleState.h"

#include "DialogueBox.h"
#include "GameEvents.h"
#include "../Engine/Animation/AnimationComponent.h"


BattleState::BattleState(GameContext& gameContext, const BattleBeginContext& battleContext):
	m_gameContext(gameContext),
	m_battleContext(battleContext)
{
	m_inputMapper.Map(SELECT_MOVE, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::Enter),
	                  static_cast<int>(sf::Keyboard::Key::Space));


	Entity& playerMonster = gameContext.m_Entities.Create<Entity>();
	playerMonster.AddComponent<EntityAnimationComponent>(
		&playerMonster,
		"MONSTER_403_BATTLE",
		EntityAnimationComponent::eAnimationName::BATTLE_BACK
	);
	playerMonster.SetScale(playerMonster.GetScale() * 2.f);

	m_playerMonsterEntityID = playerMonster.GetID();


	Entity& opponentMonster = gameContext.m_Entities.Create<Entity>();
	opponentMonster.AddComponent<EntityAnimationComponent>(
		&opponentMonster,
		"MONSTER_507_BATTLE",
		EntityAnimationComponent::eAnimationName::BATTLE_FRONT
	);
	opponentMonster.SetScale(opponentMonster.GetScale() * 2.f);

	m_opponentMonsterEntityID = opponentMonster.GetID();

	// TODO: Dynamically position them based on the sprites
	playerMonster.SetPosition({ 206, 389 });
	opponentMonster.SetPosition({ 600, 236 });
}

void BattleState::OnEnter()
{
	printf("Entered a battle!\n");

	UIMANAGER.GetUiElement("BATTLE_HUD")->OnActivate();
	DialogueBox::SetVisible(false);
}

void BattleState::OnExit()
{
	printf("Battle finished!\n");
	UIMANAGER.GetUiElement("BATTLE_HUD")->OnDeactivate();

	m_gameContext.m_Entities.Destroy(m_playerMonsterEntityID);
	m_gameContext.m_Entities.Destroy(m_opponentMonsterEntityID);
}

void BattleState::Update(const float deltaTime)
{
	m_inputMapper.Update();

	auto* playerMonster = m_gameContext.m_Entities.Get<Entity>(m_playerMonsterEntityID);
	ASSERT(playerMonster != nullptr);
	playerMonster->Update(deltaTime);

	auto* opponentMonster = m_gameContext.m_Entities.Get<Entity>(m_opponentMonsterEntityID);
	ASSERT(opponentMonster != nullptr);
	opponentMonster->Update(deltaTime);

	if (m_inputMapper.IsButtonPressed(SELECT_MOVE))
	{
		game_events::OnBattleEnd.Fire({
			.m_LevelHash = m_battleContext.m_LevelHash,
			.m_PlayerPosition = m_battleContext.m_PlayerPosition,
		});
	}
}

void BattleState::Render(sf::RenderWindow& window) const
{
	UIMANAGER.RenderBackground(window);

	Entity* playerMonster = m_gameContext.m_Entities.Get<Entity>(m_playerMonsterEntityID);
	if (playerMonster != nullptr)
	{
		m_gameContext.m_Renderer.RenderEntity(window, playerMonster);
	}

	Entity* opponentMonster = m_gameContext.m_Entities.Get<Entity>(m_opponentMonsterEntityID);
	if (opponentMonster != nullptr)
	{
		m_gameContext.m_Renderer.RenderEntity(window, opponentMonster);
	}

	UIMANAGER.RenderMidground(window);
	UIMANAGER.RenderForeground(window);
}
