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

	// TODO: I hate this being on code side, but can't think of a cleaner way at the moment...
	constexpr sf::Vector2i STATUS_ICON_SIZE{ 20, 8 };
	sf::IntRect bounds;
	switch (effect)
	{
	case Toxic:
	case Poison:
		bounds = { { 0, 0 }, STATUS_ICON_SIZE };
		break;
	case Paralysis:
		bounds = { { 20, 0 }, STATUS_ICON_SIZE };
		break;
	case Sleep:
		bounds = { { 40, 0 }, STATUS_ICON_SIZE };
		break;
	case Burn:
		bounds = { { 0, 8 }, STATUS_ICON_SIZE };
		break;
	case Freeze:
		bounds = { { 20, 8 }, STATUS_ICON_SIZE };
		break;
	default:
		ASSERT(false, "Unsupported status condition %d. Make sure it's a non-volatile condition!", static_cast<int>(effect));
		return;
	}

	conditionIcon->SetTexture(HASH("STATUS_ICONS"), bounds);
	conditionIcon->OnActivate();
}
