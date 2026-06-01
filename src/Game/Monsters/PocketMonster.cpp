#include "PocketMonster.h"

#include <iostream>

#include "../../Engine/Stringtable.h"

MonsterStats::MonsterStats(
	const uint16_t hp,
	const uint16_t attack,
	const uint16_t defense,
	const uint16_t spAttack,
	const uint16_t spDefense,
	const uint16_t speed) :
	m_HP(hp),
	m_Attack(attack),
	m_Defense(defense),
	m_SpAttack(spAttack),
	m_SpDefense(spDefense),
	m_Speed(speed)
{
}

void MonsterStats::Log() const
{
	std::cout << "Monster Stats:\n"
		"- HP: " << m_HP <<
		"\n- Attack: " << m_Attack <<
		"\n- Defense: " << m_Defense <<
		"\n- SpAttack: " << m_SpAttack <<
		"\n- SpDefense: " << m_SpDefense <<
		"\n- Speed: " << m_Speed << "\n";
}

MonsterStats MonsterStats::GetNextStat(const MonsterStats base,
                                       const uint8_t level,
                                       const uint64_t EVs,
                                       const uint64_t IVs)
{
	return {
		GetNextStat(base, HP, level, EVs, IVs),
		GetNextStat(base, Attack, level, EVs, IVs),
		GetNextStat(base, Defense, level, EVs, IVs),
		GetNextStat(base, SpecialAttack, level, EVs, IVs),
		GetNextStat(base, SpecialDefense, level, EVs, IVs),
		GetNextStat(base, Speed, level, EVs, IVs)
	};
}

uint16_t MonsterStats::GetNextStat(const MonsterStats base,
                                   const eStat stat,
                                   const uint8_t level,
                                   const uint64_t EVs,
                                   const uint64_t IVs)
{
	const uint16_t ev = (EVs >> stat) & 0xFF;
	const uint16_t iv = (IVs >> stat) & 0xFF;

	uint16_t statVal = 0;
	switch (stat)
	{
	case HP:
		statVal = base.m_HP;
		break;
	case Attack:
		statVal = base.m_Attack;
		break;
	case Defense:
		statVal = base.m_Defense;
		break;
	case SpecialAttack:
		statVal = base.m_SpAttack;
		break;
	case SpecialDefense:
		statVal = base.m_SpDefense;
		break;
	case Speed:
		statVal = base.m_Speed;
		break;
	}

	const uint16_t lhs = ((2 * statVal + iv + (ev / 4)) * level) / 100;

	// https://bulbapedia.bulbagarden.net/wiki/Stat
	if (stat == HP)
	{
		return lhs + level + 10;
	}
	else
	{
		return lhs + 5;
	}
}

PocketMonster::PocketMonster(const uint32_t id,
                             const monster_type type,
                             const MonsterStats& stats,
                             const evolution_tree& prevEvolutions,
                             const evolution_tree& nextEvolutions) :
	m_id(id),

#if BUILD_DEBUG
	m_monsterName(StringTable::Get()->GetString(GetName())),
#endif

	m_type(type),
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
