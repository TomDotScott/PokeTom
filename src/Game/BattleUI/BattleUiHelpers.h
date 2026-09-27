#ifndef STATUSICONS_H
#define STATUSICONS_H

#include "../StatusEffects.h"
#include "../../Engine/Hash.h"

namespace status_icon_utils
{
	void ClearStatusIcon(bool isPlayerIcon);
	void UpdateStatusIcon(eStatusEffect effect, bool isPlayerIcon);
}

namespace panel_names
{
	constexpr const char* BATTLE_PANEL = "BATTLE_HUD_PANEL";
	constexpr const char* OPTIONS_PANEL = "BATTLE_OPTIONS";
	constexpr const char* MOVES_PANEL = "MOVE_OPTIONS";
}

namespace text_names
{
	constexpr const char* BATTLE_TEXT = "TEXT_BOX_TEXT";
}

namespace sprite_names
{
	constexpr const char* PLAYER_STATUS_ICON = "PLAYER_STATUS_ICON";
	constexpr const char* OPPONENT_STATUS_ICON = "OPPONENT_STATUS_ICON";
}

namespace stringtable_groups
{
	const static hash_type MONSTER_NAME = HASH("MONSTER_NAME");
}

#endif // STATUSICONS_H
