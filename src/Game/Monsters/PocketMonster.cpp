#include "PocketMonster.h"

#include "../../Engine/Stringtable.h"


PocketMonster::PocketMonster() = default;

PocketMonster::PocketMonster(const uint64_t id, const monster_type type, const MonsterStats stats,
                             const evolution_tree& prevEvolutions, const evolution_tree& nextEvolutions) :
	m_id(id),

#if BUILD_DEBUG
	m_monsterName(StringTable::Get()->GetString(GetNameID())),
#endif

	m_type(type),
	m_nextEvolutions(nextEvolutions),
	m_prevEvolutions(prevEvolutions),
	m_stats(stats)
{
}

hash_type PocketMonster::GetNameID() const
{
	std::string id = "MONSTER_" + std::to_string(m_id);
	return HASH(id);
}
