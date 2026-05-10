#include "BattleState.h"

#include "DialogueBox.h"
#include "GameEvents.h"


BattleState::BattleState(GameContext& gameContext, const BattleContext& battleContext):
	m_gameContext(gameContext),
	m_battleContext(battleContext)
{
	m_inputMapper.Map(SELECT_MOVE, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::Enter), static_cast<int>(sf::Keyboard::Key::Space));
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
}

void BattleState::Update(float deltaTime)
{
	m_inputMapper.Update();


	if (m_inputMapper.IsButtonPressed(SELECT_MOVE))
	{
		game_events::OnBattleEnd.Fire();
	}
}

void BattleState::Render(sf::RenderWindow& window) const
{
}
