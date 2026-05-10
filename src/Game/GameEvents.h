#ifndef GAMEEVENTS_H
#define GAMEEVENTS_H
#include "BattleContext.h"
#include "../Engine/Event.h"

namespace game_events
{
	inline Event OnScreenFadeTriggered = Event();
	inline Event OnScreenFaded = Event();

	inline Event<BattleContext> OnBattleStart = Event<BattleContext>();
	inline Event OnBattleEnd = Event(/*TODO: Pass through any items, money and stat changes here?*/);
}

#endif // GAMEEVENTS_H
