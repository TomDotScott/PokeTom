#ifndef MONSTERSTATS_H
#define MONSTERSTATS_H
#include <array>
#include <cstdint>

static constexpr unsigned STAT_COUNT = 6;

struct MonsterStats
{
	enum class eStat
	{
		HP,
		Attack,
		Defense,
		SpecialAttack,
		SpecialDefense,
		Speed,
		Accuracy,
		Evasion,
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
	uint8_t m_Accuracy; // 0-100
	uint8_t m_Evasion; // 0-100

	uint64_t GetTotal() const
	{
		return m_HP + m_Attack + m_Defense + m_SpAttack + m_SpDefense + m_Speed;
	}

	static MonsterStats GetNextStat(MonsterStats base,
	                                uint8_t level,
	                                const std::array<unsigned char, STAT_COUNT>& EVs,
	                                const std::array<unsigned char, STAT_COUNT>& IVs
	);

	static uint16_t GetNextStat(MonsterStats base,
	                            eStat stat,
	                            uint8_t level,
	                            const std::array<unsigned char, STAT_COUNT>& EVs,
	                            const std::array<unsigned char, STAT_COUNT>& IVs
	);
};


#endif // MONSTERSTATS_H
