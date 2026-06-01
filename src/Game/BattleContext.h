#ifndef BATTLECONTEXT_H
#define BATTLECONTEXT_H
#include <cstdint>

#include "../Engine/EntityRegistry.h"
#include "../Engine/Hash.h"
#include "Monsters/PocketMonster.h"

struct BattleBeginContext
{
	hash_type m_LevelHash;
	sf::Vector2f m_PlayerPosition;

	uint32_t m_PlayerEntityID;
	uint32_t m_OpponentEntityID;

	bool m_isTrainerBattle;

	std::vector<entity_id_t> m_PlayerMonsterEntityIDs;
	std::vector<PocketMonster> m_opponentMonsters;
	std::vector<uint8_t> m_opponentMonsterLevels;
};


struct BattleEndContext
{
	hash_type m_LevelHash;
	sf::Vector2f m_PlayerPosition;

	// TODO: Pass through any items won, money gained, stat changes, etc...
};

#endif // BATTLECONTEXT_H
