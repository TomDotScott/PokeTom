#ifndef MONSTER_H
#define MONSTER_H
#include <cstdint>

#include "MonsterTypes.h"
#include "../../Engine/Hash.h"

struct MonsterStats
{
	enum eStat
	{
		HP = 0,
		Attack = 8,
		Defense = 16,
		SpecialAttack = 24,
		SpecialDefense = 32,
		Speed = 64
	};

	MonsterStats() = default;

	MonsterStats(uint16_t hp,
	             uint16_t attack,
	             uint16_t defense,
	             uint16_t spAttack,
	             uint16_t spDefense,
	             uint16_t speed);

	void Log() const;


	uint16_t m_HP;
	uint16_t m_Attack;
	uint16_t m_Defense;
	uint16_t m_SpAttack;
	uint16_t m_SpDefense;
	uint16_t m_Speed;

	uint64_t GetTotal() const
	{
		return m_HP + m_Attack + m_Defense + m_SpAttack + m_SpDefense + m_Speed;
	}

	static MonsterStats GetNextStat(MonsterStats base, uint8_t level, uint64_t EVs, uint64_t IVs);
	static uint16_t GetNextStat(MonsterStats base, eStat stat, uint8_t level, uint64_t EVs, uint64_t IVs);
};

// Size 7 as there are 7 eevee-lutions in Gen5!
using evolution_tree = std::array<int, 7>;
#define DEFAULT_EVOLUTION_TREE { -1, -1, -1, -1, -1, -1, -1 }

class PocketMonster
{
public:
	PocketMonster(uint32_t id, monster_type type, const MonsterStats& stats, const evolution_tree& prevEvolutions,
	              const evolution_tree& nextEvolutions);

	uint32_t GetID() const;
	hash_type GetName() const;

	const MonsterStats& GetBaseStats() const;

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
