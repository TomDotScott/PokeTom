#include "OptionSelectLayer.h"

#include "../BattleState.h"
#include "../../Engine/UI/UiManager.h"
#include "../../Engine/UI/UiPanel.h"
#include "../../Engine/UI/UiProgressBar.h"
#include "../../Engine/UI/UiText.h"
#include "../../Game/Monsters/PocketMonsterEntity.h"

OptionSelectLayer::OptionSelectLayer() :
	UILayer(),
	m_selectedOption()
{
}

UILayer::LayerResult OptionSelectLayer::GetLayerResult() const
{
	switch (m_selectedOption)
	{
	case eSelectedOption::Fight:
		return { .m_NextLayer = MoveSelect };
	case eSelectedOption::Monsters:
		return { .m_NextLayer = MonsterSelect };
	case eSelectedOption::Bag:
		return { .m_NextLayer = ItemSelect };
	case eSelectedOption::Run:
		return { .m_NextLayer = QuitBattle };
	}

	return { .m_NextLayer = OptionSelect };
}

void OptionSelectLayer::OnNavigateButtonPressed(const eUILayerNavigateButtons button)
{
	const bool upDown = button & (UP | DOWN);
	const bool leftRight = button & (LEFT | RIGHT);

	// FIGHT       BAG
	// POKEMON     RUN
	if (upDown)
	{
		switch (m_selectedOption)
		{
		case eSelectedOption::Fight:
			OnSelectedOptionChanged(eSelectedOption::Monsters);
			break;
		case eSelectedOption::Monsters:
			OnSelectedOptionChanged(eSelectedOption::Fight);
			break;
		case eSelectedOption::Bag:
			OnSelectedOptionChanged(eSelectedOption::Run);
			break;
		case eSelectedOption::Run:
			OnSelectedOptionChanged(eSelectedOption::Bag);
			break;
		}
	}
	else if (leftRight)
	{
		switch (m_selectedOption)
		{
		case eSelectedOption::Fight:
			OnSelectedOptionChanged(eSelectedOption::Bag);
			break;
		case eSelectedOption::Monsters:
			OnSelectedOptionChanged(eSelectedOption::Run);
			break;
		case eSelectedOption::Bag:
			OnSelectedOptionChanged(eSelectedOption::Fight);
			break;
		case eSelectedOption::Run:
			OnSelectedOptionChanged(eSelectedOption::Monsters);
			break;
		}
	}
}

void OptionSelectLayer::OnSelectButtonPressed()
{
	m_finished = true;
}

void OptionSelectLayer::OnActivate(const BattleState& state, const LayerResult& prevLayerResult)
{
	UILayer::OnActivate(state, prevLayerResult);

	const auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	auto* optionsUI = battleUI->GetChild<UiPanel>(OPTIONS_PANEL_NAME);
	ASSERT(optionsUI != nullptr);

	auto* moveUI = battleUI->GetChild<UiPanel>(MOVES_PANEL_NAME);
	ASSERT(moveUI != nullptr);

	optionsUI->OnActivate();
	moveUI->OnDeactivate();

	// Ensure the healthbars show the right value
	UiProgressBar* playerPB = battleUI->GetChild<UiProgressBar>("PLAYER_HP_BAR");
	UiProgressBar* oppPB = battleUI->GetChild<UiProgressBar>("OPPONENT_HP_BAR");

	ASSERT(playerPB && oppPB);

	const auto& entReg = state.GetGameContext().m_Entities;

	PocketMonsterEntity* playerEntity = entReg.Get<PocketMonsterEntity>(state.GetPlayerMonsterEntityID());
	ASSERT(playerEntity);
	const uint16_t playerHP = playerEntity->GetStats().m_HP;
	const uint16_t playerMaxHP = playerEntity->GetMaxHP();
	playerPB->SetProgress(playerHP, playerMaxHP, false);

	auto* hpText= battleUI->GetChild<UiText>("PLAYER_HP_TEXT");
	ASSERT(hpText);
	hpText->SetText("%d/%d", playerHP, playerMaxHP);


	PocketMonsterEntity* oppEntity = entReg.Get<PocketMonsterEntity>(state.GetOpponentMonsterEntityID());
	ASSERT(oppEntity);
	oppPB->SetProgress(oppEntity->GetStats().m_HP, oppEntity->GetMaxHP(), false);

	OnSelectedOptionChanged(eSelectedOption::Fight);
}

void OptionSelectLayer::OnDeactivate()
{
	UILayer::OnDeactivate();

	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	auto* optionsUI = dynamic_cast<UiPanel*>(battleUI->GetChild(OPTIONS_PANEL_NAME));
	ASSERT(optionsUI != nullptr);

	optionsUI->OnDeactivate();
}

void OptionSelectLayer::OnSelectedOptionChanged(const eSelectedOption newOption)
{
	m_selectedOption = newOption;

	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	auto* optionsUI = dynamic_cast<UiPanel*>(battleUI->GetChild(OPTIONS_PANEL_NAME));
	ASSERT(optionsUI != nullptr);

	auto* fightArrow = optionsUI->GetChild("FIGHT_ARROW");
	ASSERT(fightArrow != nullptr);

	auto* monstersArrow = optionsUI->GetChild("MONSTERS_ARROW");
	ASSERT(monstersArrow != nullptr);

	auto* bagArrow = optionsUI->GetChild("BAG_ARROW");
	ASSERT(bagArrow != nullptr);

	auto* runArrow = optionsUI->GetChild("RUN_ARROW");
	ASSERT(runArrow != nullptr);

	fightArrow->OnDeactivate();
	monstersArrow->OnDeactivate();
	bagArrow->OnDeactivate();
	runArrow->OnDeactivate();

	switch (newOption)
	{
	case eSelectedOption::Fight:
		fightArrow->OnActivate();
		break;
	case eSelectedOption::Monsters:
		monstersArrow->OnActivate();
		break;
	case eSelectedOption::Bag:
		bagArrow->OnActivate();
		break;
	case eSelectedOption::Run:
		runArrow->OnActivate();
		break;
	}
}
