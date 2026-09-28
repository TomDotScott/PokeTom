#include "BattleUiHelpers.h"

#include "../../Engine/UI/UiManager.h"
#include "../../Engine/UI/UiPanel.h"
#include "../../Engine/UI/UiSprite.h"

void status_icon_utils::ClearStatusIcon(const bool isPlayerIcon)
{
	const UiPanel* panel = UIMANAGER.GetElement<UiPanel>(panel_names::BATTLE_PANEL);
	panel->GetChild(isPlayerIcon ? sprite_names::PLAYER_STATUS_ICON : sprite_names::OPPONENT_STATUS_ICON)->OnDeactivate();
}

void status_icon_utils::UpdateStatusIcon(const eStatusEffect effect, const bool isPlayerIcon)
{
	if (effect == NoStatus)
	{
		ClearStatusIcon(isPlayerIcon);
		return;
	}

	const UiPanel* panel = UIMANAGER.GetElement<UiPanel>(panel_names::BATTLE_PANEL);
	UiSprite* conditionIcon = panel->GetChild<UiSprite>(isPlayerIcon
		                                                    ? sprite_names::PLAYER_STATUS_ICON
		                                                    : sprite_names::OPPONENT_STATUS_ICON);
	ASSERT(conditionIcon);

	std::string conditionEffectName;
	switch (effect)
	{
	case Toxic:
	case Poison:
		conditionEffectName = "Poison";
		break;
	case Paralysis:
		conditionEffectName = "Paralysis";
		break;
	case Sleep:
		conditionEffectName = "Sleep";
		break;
	case Burn:
		conditionEffectName = "Burn";
		break;
	case Freeze:
		conditionEffectName = "Freeze";
		break;
	default:
		ASSERT(false, "Unsupported status condition %d. Make sure it's a non-volatile condition!", static_cast<int>(effect));
		return;
	}

	conditionIcon->SetTexture(conditionEffectName);
	conditionIcon->OnActivate();
}
