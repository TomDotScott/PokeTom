#ifndef MONSTER_H
#define MONSTER_H
#include <cstdint>

#include "MonsterTypes.h"
#include "../../Engine/Hash.h"

struct MonsterStats
{
	uint8_t m_HP;
	uint8_t m_Attack;
	uint8_t m_Defense;
	uint8_t m_SpAttack;
	uint8_t m_SpDefense;
	uint8_t m_Speed;


	uint64_t GetTotal() const
	{
		return m_HP + m_Attack + m_Defense + m_SpAttack + m_SpDefense + m_Speed;
	}
};

// Size 7 as there are 7 eevee-lutions in Gen5!
using evolution_tree = std::array<int, 7>;
#define DEFAULT_EVOLUTION_TREE { -1, -1, -1, -1, -1, -1, -1 }

class PocketMonster
{
public:
	PocketMonster();
	PocketMonster(uint64_t id, monster_type type, MonsterStats stats, const evolution_tree& prevEvolutions, const evolution_tree& nextEvolutions);

	hash_type GetNameID() const;

private:
	uint64_t m_id;

#if BUILD_DEBUG
	std::string m_monsterName;
#endif

	monster_type m_type;

	evolution_tree m_nextEvolutions;
	evolution_tree m_prevEvolutions;

	MonsterStats m_stats;
};


#endif
