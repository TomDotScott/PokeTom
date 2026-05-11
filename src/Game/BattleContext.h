#ifndef BATTLECONTEXT_H
#define BATTLECONTEXT_H
#include <cstdint>

#include "../Engine/Hash.h"

struct BattleBeginContext
{
	hash_type m_LevelHash;
	sf::Vector2f m_PlayerPosition;

	uint32_t m_PlayerEntityID;
	uint32_t m_OpponentEntityID;

	// TODO: Party details, monster level + species, etc...
};


struct BattleEndContext
{
	hash_type m_LevelHash;
	sf::Vector2f m_PlayerPosition;

	// TODO: Pass through any items won, money gained, stat changes, etc...
};

#endif // BATTLECONTEXT_H
