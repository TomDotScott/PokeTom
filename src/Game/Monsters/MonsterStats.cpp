#include "MonsterStats.h"

#include <iostream>

#include "../../Engine/Maths.h"

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

MonsterStatComponent::MonsterStatComponent(const MonsterStats& currentStats) :
	m_statModifiers{ 0, 0, 0, 0, 0, 0 },
	m_stats{ currentStats }
{
}

void MonsterStatComponent::Update(const float deltaTime)
{
}

void MonsterStatComponent::ResetAllModifiers()
{
	m_statModifiers = { 0, 0, 0, 0, 0, 0 };
}

void MonsterStatComponent::ResetModifier(MonsterStats::eStat stat)
{
	m_statModifiers[static_cast<size_t>(stat)] = 0;
}

void MonsterStatComponent::ModifyStat(MonsterStats::eStat stat, const int stages)
{
	int& currentStage = m_statModifiers[static_cast<size_t>(stat)];
	currentStage = maths::Clamp(currentStage + stages, -6, 6);
}

bool MonsterStatComponent::IsFainted() const
{
	return m_stats.m_HP == 0;
}

MonsterStats& MonsterStatComponent::GetStats()
{
	return m_stats;
}
