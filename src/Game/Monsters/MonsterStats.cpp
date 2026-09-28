#include "MonsterStats.h"

#include <iostream>

#include "../../Engine/Maths.h"
#include "../../Engine/Stringtable.h"

namespace
{
	// Taken from https://bulbapedia.bulbagarden.net/wiki/Stat_modifier
	constexpr std::array STAT_MULTIPLIERS = {
		2.f / 8.f,
		2.f / 7.f,
		2.f / 6.f,
		2.f / 5.f,
		2.f / 4.f,
		2.f / 3.f,
		2.f / 2.f,
		3.f / 2.f,
		4.f / 2.f,
		5.f / 2.f,
		6.f / 2.f,
		7.f / 2.f,
		8.f / 2.f,
	};

	constexpr std::array ACCURACY_EVASION_MULTIPLIERS = {
		3.f / 9.f,
		3.f / 8.f,
		3.f / 7.f,
		3.f / 6.f,
		3.f / 5.f,
		3.f / 4.f,
		3.f / 3.f,
		4.f / 3.f,
		5.f / 3.f,
		6.f / 3.f,
		7.f / 3.f,
		8.f / 3.f,
		9.f / 3.f,
	};
}

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
	m_Speed(speed),
	m_Accuracy(100),
	m_Evasion(100)
{
}

void MonsterStats::TakeDamage(const monster_hp_t damage)
{
	if (damage > m_HP)
	{
		m_HP = 0;
	}
	else
	{
		m_HP -= damage;
	}
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
                                       const std::array<unsigned char, STAT_COUNT>& EVs,
                                       const std::array<unsigned char, STAT_COUNT>& IVs)
{
	return {
		GetNextStat(base, eStat::HP, level, EVs, IVs),
		GetNextStat(base, eStat::Attack, level, EVs, IVs),
		GetNextStat(base, eStat::Defense, level, EVs, IVs),
		GetNextStat(base, eStat::SpecialAttack, level, EVs, IVs),
		GetNextStat(base, eStat::SpecialDefense, level, EVs, IVs),
		GetNextStat(base, eStat::Speed, level, EVs, IVs)
	};
}

uint16_t MonsterStats::GetNextStat(const MonsterStats base,
                                   const eStat stat,
                                   const uint8_t level,
                                   const std::array<unsigned char, STAT_COUNT>& EVs,
                                   const std::array<unsigned char, STAT_COUNT>& IVs)
{
	const size_t idx = static_cast<size_t>(stat);

	const uint16_t ev = EVs[idx];
	const uint16_t iv = IVs[idx];

	uint16_t statVal = 0;
	switch (stat)
	{
	case eStat::HP:
		statVal = base.m_HP;
		break;
	case eStat::Attack:
		statVal = base.m_Attack;
		break;
	case eStat::Defense:
		statVal = base.m_Defense;
		break;
	case eStat::SpecialAttack:
		statVal = base.m_SpAttack;
		break;
	case eStat::SpecialDefense:
		statVal = base.m_SpDefense;
		break;
	case eStat::Speed:
		statVal = base.m_Speed;
		break;
	default:
		return 100;
	}

	const uint16_t lhs = ((2 * statVal + iv + (ev / 4)) * level) / 100;

	// https://bulbapedia.bulbagarden.net/wiki/Stat
	if (stat == eStat::HP)
	{
		return lhs + level + 10;
	}
	else
	{
		return lhs + 5;
	}
}

std::string MonsterStats::GetStatString(eStat stat)
{
	const hash_type stGroup = HASH("STAT");

	switch (stat)
	{
	case eStat::HP:
		return STRINGTABLE->GetString(stGroup, HASH("STAT_HP"));
	case eStat::Attack:
		return STRINGTABLE->GetString(stGroup, HASH("STAT_ATTACK"));
	case eStat::Defense:
		return STRINGTABLE->GetString(stGroup, HASH("STAT_DEFENSE"));
	case eStat::SpecialAttack:
		return STRINGTABLE->GetString(stGroup, HASH("STAT_ATTACK_SPECIAL"));
	case eStat::SpecialDefense:
		return STRINGTABLE->GetString(stGroup, HASH("STAT_DEFENSE_SPECIAL"));
	case eStat::Speed:
		return STRINGTABLE->GetString(stGroup, HASH("STAT_SPEED"));
	case eStat::Accuracy:
		return STRINGTABLE->GetString(stGroup, HASH("STAT_ACCURACY"));
	case eStat::Evasion:
		return STRINGTABLE->GetString(stGroup, HASH("STAT_EVASION"));
	}

	ASSERT_MSG(false, "Unknown stat %d", static_cast<int>(stat));
	return "UNKNOWN_STAT";
}

MonsterStatComponent::MonsterStatComponent(const MonsterStats& currentStats) :
	m_statModifiers{ 0, 0, 0, 0, 0, 0, 0, 0 },
	m_stats{ currentStats },
	m_maxHP(m_stats.m_HP)
{
}

void MonsterStatComponent::Update(const float deltaTime)
{
}

void MonsterStatComponent::FullHeal(const MonsterStats& baseStats,
                                    const uint8_t currentLevel,
                                    const std::array<uint8_t, STAT_COUNT>& IVs,
                                    const std::array<uint8_t, STAT_COUNT>& EVs)
{
	m_stats = MonsterStats::GetNextStat(baseStats, currentLevel, EVs, IVs);
}

void MonsterStatComponent::ResetAllModifiers()
{
	m_statModifiers = { 0, 0, 0, 0, 0, 0, 0, 0 };
}

void MonsterStatComponent::ResetModifier(MonsterStats::eStat stat)
{
	m_statModifiers[static_cast<size_t>(stat)] = 0;
}

bool MonsterStatComponent::ModifyStat(MonsterStats::eStat stat, const int stages)
{
	int& currentStage = m_statModifiers[static_cast<size_t>(stat)];
	const int previousStage = currentStage;
	currentStage = maths::Clamp(currentStage + stages, -6, 6);

	return currentStage != previousStage;
}

bool MonsterStatComponent::IsFainted() const
{
	return m_stats.m_HP == 0;
}

MonsterStats MonsterStatComponent::GetStats() const
{
	MonsterStats stats;

	stats.m_HP = GetModifiedStatValue(MonsterStats::eStat::HP);
	stats.m_Accuracy = static_cast<uint8_t>(GetModifiedStatValue(MonsterStats::eStat::Accuracy));
	stats.m_Attack = GetModifiedStatValue(MonsterStats::eStat::Attack);
	stats.m_Defense = GetModifiedStatValue(MonsterStats::eStat::Defense);
	stats.m_Evasion = static_cast<uint8_t>(GetModifiedStatValue(MonsterStats::eStat::Evasion));
	stats.m_SpAttack = GetModifiedStatValue(MonsterStats::eStat::SpecialAttack);
	stats.m_SpDefense = GetModifiedStatValue(MonsterStats::eStat::SpecialDefense);
	stats.m_Speed = GetModifiedStatValue(MonsterStats::eStat::Speed);

	return stats;
}

monster_hp_t MonsterStatComponent::GetMaxHP() const
{
	return m_maxHP;
}

void MonsterStatComponent::TakeDamage(const monster_hp_t damage)
{
	m_stats.TakeDamage(damage);
}

void MonsterStatComponent::GrowToLevel(
	const uint8_t newLevel,
	const MonsterStats& baseStats,
	const std::array<uint8_t, STAT_COUNT>& IVs,
	const std::array<uint8_t, STAT_COUNT>& EVs
)
{
	const monster_hp_t before = m_stats.m_HP;

	m_stats = MonsterStats::GetNextStat(baseStats, newLevel, EVs, IVs);
	m_maxHP = m_stats.m_HP;

	m_stats.m_HP = before;
}

uint16_t MonsterStatComponent::GetModifiedStatValue(MonsterStats::eStat stat) const
{
	const int modifier = m_statModifiers[static_cast<size_t>(stat)];

	float val;
	switch (stat)
	{
	case MonsterStats::eStat::HP:
		val = static_cast<float>(m_stats.m_HP);
		break;
	case MonsterStats::eStat::Attack:
		val = static_cast<float>(m_stats.m_Attack) * static_cast<float>(STAT_MULTIPLIERS[modifier + 6]);
		break;
	case MonsterStats::eStat::Defense:
		val = static_cast<float>(m_stats.m_Defense) * static_cast<float>(STAT_MULTIPLIERS[modifier + 6]);
		break;
	case MonsterStats::eStat::SpecialAttack:
		val = static_cast<float>(m_stats.m_SpAttack) * static_cast<float>(STAT_MULTIPLIERS[modifier + 6]);
		break;
	case MonsterStats::eStat::SpecialDefense:
		val = static_cast<float>(m_stats.m_SpDefense) * static_cast<float>(STAT_MULTIPLIERS[modifier + 6]);
		break;
	case MonsterStats::eStat::Speed:
		val = static_cast<float>(m_stats.m_Speed) * static_cast<float>(STAT_MULTIPLIERS[modifier + 6]);
		break;
	case MonsterStats::eStat::Accuracy:
		val = static_cast<float>(m_stats.m_Accuracy) * static_cast<float>(ACCURACY_EVASION_MULTIPLIERS[modifier + 6]);
		break;
	case MonsterStats::eStat::Evasion:
		val = static_cast<float>(m_stats.m_Evasion) * static_cast<float>(ACCURACY_EVASION_MULTIPLIERS[modifier + 6]);
		break;
	}

	return static_cast<uint16_t>(std::round(val));
}
