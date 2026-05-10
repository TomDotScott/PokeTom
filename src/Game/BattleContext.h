#ifndef BATTLECONTEXT_H
#define BATTLECONTEXT_H
#include <cstdint>

struct BattleContext
{
	uint32_t m_PlayerEntityID;
	uint32_t m_OpponentEntityID;

	// TODO: Party details, monster level + species, etc...
};

#endif // BATTLECONTEXT_H
