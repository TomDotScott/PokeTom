#include "MonsterXP.h"

#include <cmath>
#include <cstdint>
#include <utility>

#include "../../Engine/Maths.h"

namespace
{
	template <auto Func>
	struct XpLookupTable
	{
		using value_t = decltype(Func(std::declval<uint8_t>()));

		static constexpr std::array<value_t, monster_xp::LEVEL_COUNT> generate()
		{
			return []<monster_xp_t... I>(std::index_sequence<I...>)
			{
				return std::array<value_t, monster_xp::LEVEL_COUNT>{ Func(static_cast<uint8_t>(I + 1))... };
			}(std::make_index_sequence<monster_xp::LEVEL_COUNT>{});
		}

		static constexpr std::array<value_t, monster_xp::LEVEL_COUNT> value = generate();
	};

	constexpr monster_xp_t CALCULATE_XP_ERRATIC(const uint8_t monsterLevel)
	{
		const float castedLevel = monsterLevel;
		const float levelCubed = castedLevel * castedLevel * castedLevel;

		if (monsterLevel < 50)
			return static_cast<monster_xp_t>(levelCubed * ((100.f - castedLevel) / 50.f));
		if (monsterLevel < 68)
			return static_cast<monster_xp_t>(levelCubed * (150.f - castedLevel) / 50.f);
		if (monsterLevel < 98)
			return static_cast<monster_xp_t>(levelCubed * ((1911.f - 10.f * castedLevel) * 3.f) / 500.f);

		return static_cast<monster_xp_t>(levelCubed * (160.f - castedLevel) / 100.f);
	}

	constexpr monster_xp_t CALCULATE_XP_FAST(const uint8_t monsterLevel)
	{
		const monster_xp_t castedLevel = monsterLevel;
		return 4 * castedLevel * castedLevel * castedLevel / 5;
	}

	constexpr monster_xp_t CALCULATE_XP_MEDIUM_FAST(const uint8_t monsterLevel)
	{
		const monster_xp_t castedLevel = monsterLevel;
		return castedLevel * castedLevel * castedLevel;
	}

	constexpr monster_xp_t CALCULATE_XP_MEDIUM_SLOW(const uint8_t monsterLevel)
	{
		const monster_xp_t castedLevel = monsterLevel;
		const monster_xp_t levelSquared = castedLevel * castedLevel;
		const monster_xp_t levelCubed = levelSquared * castedLevel;

		return (6 * levelCubed / 5) - (15 * levelSquared) + (100 * castedLevel) - 140;
	}

	constexpr monster_xp_t CALCULATE_XP_SLOW(const uint8_t monsterLevel)
	{
		const monster_xp_t castedLevel = monsterLevel;
		return 5 * castedLevel * castedLevel * castedLevel / 4;
	}

	constexpr monster_xp_t CALCULATE_XP_FLUCTUATING(const uint8_t monsterLevel)
	{
		const monster_xp_t castedLevel = monsterLevel;
		const monster_xp_t levelCubed = castedLevel * castedLevel * castedLevel;

		const monster_xp_t numerator = castedLevel < 15
			                               ? (castedLevel + 1) / 3 + 24
			                               : castedLevel < 36
			                               ? castedLevel + 14
			                               : castedLevel / 2 + 32;

		return levelCubed * numerator / 50;
	}
}

const std::array<std::array<monster_xp_t, monster_xp::LEVEL_COUNT>, static_cast<size_t>(monster_xp::eGroup::COUNT)>
monster_xp::XP_LOOKUP_TABLE{
	XpLookupTable<CALCULATE_XP_ERRATIC>::value,
	XpLookupTable<CALCULATE_XP_FAST>::value,
	XpLookupTable<CALCULATE_XP_MEDIUM_FAST>::value,
	XpLookupTable<CALCULATE_XP_MEDIUM_SLOW>::value,
	XpLookupTable<CALCULATE_XP_SLOW>::value,
	XpLookupTable<CALCULATE_XP_FLUCTUATING>::value,
};

monster_xp_t monster_xp::GetExperienceGain(
	const bool isOwnedByTrainer,
	const monster_xp_t xpYield,
	const bool isHoldingLuckyEgg,
	const float opponentLevel,
	const float playerLevel,
	const unsigned numPartakingMonsters
)
{
	const float a = isOwnedByTrainer ? 1.5f : 1.f;
	const float e = isHoldingLuckyEgg ? 1.5f : 1.f;
	constexpr float p = 1.f;
	constexpr float t = 6943.f / 4096.f;

	const float p1 = static_cast<float>(xpYield) * opponentLevel / 5;
	const float p2 = a;
	const float p3 = 1.f / static_cast<float>(numPartakingMonsters);

	const float twoLPlusTen = 2 * opponentLevel + 10;
	const float lPlusLpPlusTen = opponentLevel + playerLevel + 10;
	const float p4Numerator = std::floorf(sqrtf(twoLPlusTen) * (twoLPlusTen * twoLPlusTen));
	const float p4Denominator = std::floorf(sqrtf(lPlusLpPlusTen) * (lPlusLpPlusTen * lPlusLpPlusTen));
	const float p4 = p4Numerator / p4Denominator;

	const float lhs = p1 * p2 * p3 * p4 + 1;
	const float rhs = t * e * p;

	const float result = lhs * rhs;

	return static_cast<monster_xp_t>(maths::Clamp(result, 0.f, 100000.f));
}
