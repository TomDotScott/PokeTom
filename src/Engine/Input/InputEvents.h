#ifndef INPUTEVENTS_H
#define INPUTEVENTS_H

#include "../Event.h"

struct InputEvent
{
	Event<> m_onPressed;
	Event<> m_onReleased;
	Event<> m_onHeld;
};

#endif // INPUTEVENTS_H
