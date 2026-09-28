#ifndef STATUSEFFECTS_H
#define STATUSEFFECTS_H

#include <magic_enum.hpp>

enum eStatusEffect
{
	NoStatus = 0,
	Burn = 1 << 0,
	Freeze = 1 << 1,
	Paralysis = 1 << 2,
	Poison = 1 << 3,
	Sleep = 1 << 4,
	Confusion = 1 << 5,
	Infatuation = 1 << 6,
	Trap = 1 << 7,
	Disable = 1 << 8,
	Embargo = 1 << 9,
	HealBlock = 1 << 10,
	Ingrain = 1 << 11,
	LeechSeed = 1 << 12,
	Nightmare = 1 << 13,
	NoTypeImmunity = 1 << 14,
	PerishSong = 1 << 15,
	Silence = 1 << 16,
	Torment = 1 << 17,
	Yawn = 1 << 18,
	Toxic = 1 << 19,
};

inline constexpr unsigned NUM_STATUS_EFFECTS = 21;

constexpr bool IsNonVolatileStatus(const uint32_t statusEffectFlags)
{
	constexpr uint32_t volatileStatuses = Burn | Freeze | Paralysis | Poison | Sleep | Toxic;
	return statusEffectFlags & volatileStatuses;
}

template <>
struct magic_enum::customize::enum_range<eStatusEffect>
{
	static constexpr bool is_flags = true;
};

#endif // STATUSEFFECTS_H
