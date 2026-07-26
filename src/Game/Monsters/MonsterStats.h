#ifndef MONSTERSTATS_H
#define MONSTERSTATS_H
#include <array>
#include <cstdint>

#include "../../Engine/IUpdateable.h"

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

	void TakeDamage(uint16_t damage);

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

class MonsterStatComponent : public IUpdateable
{
public:
	MonsterStatComponent(const MonsterStats& currentStats);
	void Update(float deltaTime) override;

	void ResetAllModifiers();
	void ResetModifier(MonsterStats::eStat stat);

	void ModifyStat(MonsterStats::eStat stat, int stages);

	bool IsFainted() const;
	MonsterStats& GetStats();

	uint16_t GetMaxHP() const;
	void TakeDamage(uint16_t damage);

private:
	std::array<int, STAT_COUNT> m_statModifiers;
	MonsterStats m_stats;
	uint16_t m_maxHP;
};

#endif // MONSTERSTATS_H
