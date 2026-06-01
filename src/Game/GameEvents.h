#ifndef GAMEEVENTS_H
#define GAMEEVENTS_H
#include "BattleContext.h"
#include "../Engine/Event.h"

namespace game_events
{
	inline auto OnScreenFadeTriggered = Event();
	inline auto OnScreenFaded = Event();

	inline auto OnBattleStart = Event<BattleBeginContext>();
	inline auto OnBattleEnd = Event<BattleEndContext>();
}

#endif // GAMEEVENTS_H
