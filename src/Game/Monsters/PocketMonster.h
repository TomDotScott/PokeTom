#ifndef MONSTER_H
#define MONSTER_H
#include <cstdint>

#include "MonsterStats.h"
#include "MonsterTypes.h"
#include "../../Engine/Hash.h"

// Size 7 as there are 7 eevee-lutions in Gen5!
using evolution_tree = std::array<int, 7>;
#define DEFAULT_EVOLUTION_TREE { -1, -1, -1, -1, -1, -1, -1 }

class PocketMonster
{
public:
	PocketMonster(uint32_t id,
				  monster_type type,
				  const MonsterStats& stats,
				  const evolution_tree& prevEvolutions,
				  const evolution_tree& nextEvolutions
	);

	uint32_t GetID() const;
	hash_type GetName() const;

	const MonsterStats& GetBaseStats() const;
	uint32_t GetType() const;

private:
	uint32_t m_id;

#if BUILD_DEBUG
	std::string m_monsterName;
#endif

	monster_type m_type;

	evolution_tree m_nextEvolutions;
	evolution_tree m_prevEvolutions;

	MonsterStats m_baseStats;
};


#endif
