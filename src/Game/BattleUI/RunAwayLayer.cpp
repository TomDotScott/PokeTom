#include "RunAwayLayer.h"
#include "../BattleState.h"
#include "../DialogueBox.h"
#include "../GameEvents.h"
#include "../../Engine/UI/UiManager.h"
#include "../../Engine/Stringtable.h"


UILayer::LayerResult RunAwayLayer::GetLayerResult() const
{
	return { .m_NextLayer = OptionSelect };
}

void RunAwayLayer::OnNavigateButtonPressed(eUILayerNavigateButtons /*button*/)
{
}

void RunAwayLayer::OnSelectButtonPressed()
{
	game_events::OnBattleEnd.Fire(m_endContext);
}

void RunAwayLayer::OnBackButtonPressed()
{
}

void RunAwayLayer::OnMoreInfoButtonPressed()
{
}

void RunAwayLayer::OnActivate(const BattleState& state, const LayerResult& prevLayerResult)
{
	UILayer::OnActivate(state, prevLayerResult);

	m_endContext = {
		.m_LevelHash = state.GetBattleContext().m_LevelHash,
		.m_PlayerPosition = state.GetBattleContext().m_PlayerPosition,
	};

	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	auto* optionsUI = dynamic_cast<UiPanel*>(battleUI->GetChild(OPTIONS_PANEL_NAME));
	ASSERT(optionsUI != nullptr);

	optionsUI->OnDeactivate();

	DialogueBox::SetText("%s got away safely!\n",
	                     STRINGTABLE->GetString(HASH("CHARACTER"), HASH("PLAYER_NAME")).c_str());
	DialogueBox::SetVisible(true);
}

void RunAwayLayer::OnDeactivate()
{
	UILayer::OnDeactivate();

	DialogueBox::SetVisible(false);
	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	battleUI->OnDeactivate();
}
