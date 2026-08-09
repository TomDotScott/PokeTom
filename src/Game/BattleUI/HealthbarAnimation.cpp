#include "HealthbarAnimation.h"

#include "UILayer.h"
#include "../../Engine/UI/UiManager.h"
#include "../../Engine/UI/UiPanel.h"
#include "../../Engine/UI/UiProgressBar.h"
#include "../../Engine/UI/UiText.h"
#include "../Monsters/PocketMonsterEntity.h"

HealthbarAnimation::HealthbarAnimation(const eAnimationType type) :
	AnimatedProgressBar<monster_hp_t>(type == eAnimationType::Player
		                                  ? UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME)->GetChild<UiProgressBar>(
			                                  "PLAYER_HP_BAR")
		                                  : UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME)->GetChild<UiProgressBar>(
			                                  "OPPONENT_HP_BAR")
	),
	m_hpText(nullptr),
	m_monster(nullptr),
	m_type(type)
{
	const UiPanel* battlePanel = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	if (m_type == eAnimationType::Player)
	{
		UiText* text = battlePanel->GetChild<UiText>("PLAYER_HP_TEXT");
		ASSERT(text);
		m_hpText = text;
	}
}

void HealthbarAnimation::Start(const monster_hp_t monsterHealthBefore)
{
	ASSERT(m_monster);

	const monster_hp_t currentHP = m_monster->GetStats().m_HP;
	const monster_hp_t maxHP = m_monster->GetMaxHP();

	const float damageDealt = static_cast<float>(monsterHealthBefore) - static_cast<float>(currentHP);

	const float percentage = damageDealt / static_cast<float>(maxHP);
	bool lotsOfDamage = true;
	if (percentage > 0.25f)
	{
		lotsOfDamage = false;
	}

	const float duration = lotsOfDamage ? 3.5f : 1.8f;

	AnimatedProgressBar::Start(monsterHealthBefore, currentHP, maxHP, duration);
}

void HealthbarAnimation::UpdateFill() const
{
	AnimatedProgressBar::UpdateFill();

	if (m_type == eAnimationType::Player)
	{
		ASSERT(m_hpText);

		const float animHealth = std::round(CalculateFill() * static_cast<float>(m_maximumValue));
		m_hpText->SetText("%d/%d", static_cast<int>(animHealth), m_maximumValue);
	}
}

void HealthbarAnimation::SetMonster(PocketMonsterEntity* monster)
{
	m_monster = monster;
	m_maximumValue = monster->GetMaxHP();

	if (m_type == eAnimationType::Player)
	{
		ASSERT(m_hpText);

		m_hpText->SetText("%d/%d", monster->GetStats().m_HP, m_maximumValue);
	}
}
