#include "OptionSelectLayer.h"

#include "../BattleState.h"
#include "../../Engine/Maths.h"
#include "../../Engine/Stringtable.h"
#include "../../Engine/StringUtils.h"
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
	default:
		return { .m_NextLayer = OptionSelect };
	}
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

void OptionSelectLayer::OnBackButtonPressed()
{
	m_finished = true;
	m_selectedOption = eSelectedOption::None;
}

void OptionSelectLayer::OnMoreInfoButtonPressed()
{
	// TODO: Show a Dialog with the buttons?
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

	PocketMonsterEntity* playerMonster = entReg.Get<PocketMonsterEntity>(state.GetPlayerMonsterEntityID());
	ASSERT(playerMonster);

	auto* textBoxText = battleUI->GetChild<UiText>(BATTLE_TEXT_NAME);
	ASSERT(textBoxText != nullptr);
	textBoxText->OnActivate();

	// TODO: Monster Nicknames!
	const std::string playerMonsterName = STRINGTABLE->GetString(STRING_MONSTER_NAME_GRP, playerMonster->GetNameStringID());
	const std::string chooseOptionString = STRINGTABLE->GetDynamicString(HASH("PLAYER_CHOOSE_OPTION"), playerMonsterName.c_str());
	// TODO: Do I need to make the width data driven?
	const string_utils::text_pages chooseOptionText = string_utils::WrapToPages(chooseOptionString, 10, 3);
	ASSERT(chooseOptionText.size() == 1);
	textBoxText->SetText(chooseOptionText[0].c_str());


	const monster_hp_t playerHP = playerMonster->GetStats().m_HP;
	const monster_hp_t playerMaxHP = playerMonster->GetMaxHP();
	playerPB->SetProgress(playerHP, playerMaxHP, false);

	auto* hpText = battleUI->GetChild<UiText>("PLAYER_HP_TEXT");
	ASSERT(hpText);
	hpText->SetText("%d/%d", playerHP, playerMaxHP);

	const uint8_t currentLevel = playerMonster->GetLevel();
	auto* levelText = battleUI->GetChild<UiText>("PLAYER_LEVEL_TEXT");
	ASSERT(levelText);
	levelText->SetText("%d", currentLevel);

	auto* xpBar = battleUI->GetChild<UiProgressBar>("PLAYER_XP_BAR");
	ASSERT(xpBar);

	const monster_xp_t prevLevelXP = monster_xp::GetMaxExperienceForLevel(playerMonster->GetExperienceGroup(), currentLevel);
	const monster_xp_t nextLevelXP = GetMaxExperienceForLevel(playerMonster->GetExperienceGroup(), currentLevel + 1);
	const monster_xp_t a = playerMonster->GetCurrentXP() - prevLevelXP;
	const monster_xp_t b = nextLevelXP - prevLevelXP;
	xpBar->SetProgress(static_cast<float>(a) / static_cast<float>(b));


	PocketMonsterEntity* oppEntity = entReg.Get<PocketMonsterEntity>(state.GetOpponentMonsterEntityID());
	ASSERT(oppEntity);
	oppPB->SetProgress(oppEntity->GetStats().m_HP, oppEntity->GetMaxHP(), false);

	const uint8_t oppCurrentLevel = oppEntity->GetLevel();
	auto* oppLevelText = battleUI->GetChild<UiText>("OPPONENT_LEVEL_TEXT");
	ASSERT(oppLevelText);
	oppLevelText->SetText("%d", oppCurrentLevel);

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
