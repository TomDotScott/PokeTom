#ifndef MONSTERTYPES_H
#define MONSTERTYPES_H
#include <cstdint>
#include <magic_enum.hpp>

enum eTypeFlags
{
	NORMAL = 1 << 0,
	FIRE = 1 << 1,
	WATER = 1 << 2,
	ELECTRIC = 1 << 3,
	GRASS = 1 << 4,
	ICE = 1 << 5,
	FIGHTING = 1 << 6,
	POISON = 1 << 7,
	GROUND = 1 << 8,
	FLYING = 1 << 9,
	PSYCHIC = 1 << 10,
	BUG = 1 << 11,
	ROCK = 1 << 12,
	GHOST = 1 << 13,
	DRAGON = 1 << 14,
	STEEL = 1 << 15,
	DARK = 1 << 16,
	FAIRY = 1 << 17,
};

template <>
struct magic_enum::customize::enum_range<eTypeFlags>
{
	static constexpr bool is_flags = true;
};

inline constexpr unsigned NUM_TYPES = 18;

// eTypeFlags make up the latter 18 bits of the int
using monster_type = uint32_t;

extern constexpr float CalculateAttackingEffectiveness(monster_type attacker, monster_type defender);
extern constexpr float CalculateDefendingEffectiveness(monster_type attacker, monster_type defender);

#endif
