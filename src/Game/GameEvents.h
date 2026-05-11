#ifndef GAMEEVENTS_H
#define GAMEEVENTS_H
#include "BattleContext.h"
#include "../Engine/Event.h"

namespace game_events
{
	inline Event OnScreenFadeTriggered = Event();
	inline Event OnScreenFaded = Event();

	inline Event<BattleBeginContext> OnBattleStart = Event<BattleBeginContext>();
	inline Event<BattleEndContext> OnBattleEnd = Event<BattleEndContext>();
}

#endif // GAMEEVENTS_H
