#ifndef MONSTERXP_H
#define MONSTERXP_H

#include <algorithm>
#include <array>
#include <cstdint>

using monster_xp_t = uint64_t;

namespace monster_xp
{
	constexpr uint64_t LEVEL_COUNT = 100;

	enum class eGroup : char
	{
		ERRATIC,
		FAST,
		MEDIUM_FAST,
		MEDIUM_SLOW,
		SLOW,
		FLUCTUATING,
		COUNT
	};

	extern const std::array<std::array<monster_xp_t, LEVEL_COUNT>, static_cast<size_t>(eGroup::COUNT)> XP_LOOKUP_TABLE;

	constexpr monster_xp_t GetMaxExperienceForLevel(const eGroup group, const uint8_t level)
	{
		return XP_LOOKUP_TABLE[static_cast<size_t>(group)][static_cast<size_t>(level)];
	}

	constexpr uint8_t GetLevelForExperience(const eGroup group, const uint64_t experience)
	{
		const auto& table = XP_LOOKUP_TABLE[static_cast<size_t>(group)];

		const auto it = std::ranges::upper_bound(table, experience);

		// upper_bound gives the first level requiring MORE xp than we have,
		// so the level we're actually at is one before that.
		if (it == table.begin())
			return 1;

		return static_cast<uint8_t>(std::distance(table.begin(), it)) - 1;
	}

	monster_xp_t GetExperienceGain(bool isOwnedByTrainer, monster_xp_t xpYield, bool isHoldingLuckyEgg, float opponentLevel, float playerLevel, unsigned numPartakingMonsters);
}

#endif // MONSTERXP_H
