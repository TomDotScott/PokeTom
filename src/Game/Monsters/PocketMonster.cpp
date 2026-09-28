#include "PocketMonster.h"

#include <iostream>

#include "../../Engine/Stringtable.h"


PocketMonster::PocketMonster(const uint32_t id,
                             const monster_type type,
                             const monster_xp::eGroup experienceGroup,
                             const monster_xp_t experienceYield,
                             const MonsterStats& stats,
                             const evolution_tree& prevEvolutions,
                             const evolution_tree& nextEvolutions) :
	m_id(id),

#if BUILD_DEBUG
	m_monsterName(StringTable::Get()->GetString(GetName())),
#endif

	m_type(type),
	m_experienceGroup(experienceGroup),
	m_experienceYield(experienceYield),
	m_nextEvolutions(nextEvolutions),
	m_prevEvolutions(prevEvolutions),
	m_baseStats(stats)
{
}

uint32_t PocketMonster::GetID() const
{
	return m_id;
}

hash_type PocketMonster::GetName() const
{
	std::string id = "MONSTER_" + std::to_string(m_id);
	return HASH(id);
}

const MonsterStats& PocketMonster::GetBaseStats() const
{
	return m_baseStats;
}

uint32_t PocketMonster::GetType() const
{
	return m_type;
}

monster_xp::eGroup PocketMonster::GetExperienceGroup() const
{
	return m_experienceGroup;
}

monster_xp_t PocketMonster::GetExperienceYield() const
{
	return m_experienceYield;
}
