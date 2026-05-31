#include "MonsterTypes.h"
#include <frozen/unordered_map.h>

namespace attacking_maps
{
	constexpr frozen::unordered_map<eTypeFlags, monster_type, NUM_TYPES> SUPER_EFFECTIVE =
	{
		{ NORMAL, 0 },
		{ FIRE, BUG | STEEL | ICE | GRASS },
		{ WATER, FIRE | ROCK | GROUND },
		{ GRASS, ROCK | WATER | GROUND },
		{ ELECTRIC, FLYING | WATER },
		{ ICE, FLYING | DRAGON | ROCK | GRASS },
		{ FIGHTING, STEEL | NORMAL | ICE | DARK | ROCK },
		{ POISON, GRASS | PSYCHIC },
		{ GROUND, ELECTRIC | STEEL | POISON | FIRE | ROCK },
		{ FLYING, BUG | FIGHTING | GRASS },
		{ PSYCHIC, FIGHTING | POISON },
		{ BUG, PSYCHIC | DARK | GRASS },
		{ ROCK, FLYING | BUG | ICE | FIRE },
		{ GHOST, PSYCHIC | GHOST },
		{ DRAGON, DRAGON },
		{ DARK, PSYCHIC | GHOST },
		{ STEEL, ICE | ROCK | FAIRY },
		{ FAIRY, FIGHTING | DRAGON | DARK }
	};

	constexpr frozen::unordered_map<eTypeFlags, monster_type, NUM_TYPES> NOT_VERY_EFFECTIVE =
	{
		{ NORMAL, STEEL | ROCK },
		{ FIRE, FIRE | DRAGON | WATER | ROCK },
		{ WATER, DRAGON | WATER | GRASS },
		{ GRASS, BUG | FLYING | STEEL | POISON | FIRE | DRAGON | GRASS },
		{ ELECTRIC, ELECTRIC | DRAGON | GRASS },
		{ ICE, STEEL | ICE | FIRE | WATER },
		{ FIGHTING, BUG | PSYCHIC | FLYING | POISON | FAIRY },
		{ POISON, GHOST | POISON | GROUND | ROCK },
		{ GROUND, BUG | GRASS },
		{ FLYING, ELECTRIC | STEEL | ROCK },
		{ PSYCHIC, PSYCHIC | STEEL },
		{ BUG, FIGHTING | GHOST | FLYING | STEEL | POISON | FIRE | FAIRY },
		{ ROCK, FIGHTING | STEEL | GROUND },
		{ GHOST, DARK },
		{ DRAGON, STEEL },
		{ DARK, FIGHTING | DARK | FAIRY },
		{ STEEL, FIRE | WATER | ELECTRIC | STEEL },
		{ FAIRY, FIRE | POISON | STEEL }
	};

	constexpr frozen::unordered_map<eTypeFlags, monster_type, 8> IMMUNE =
	{
		{ NORMAL, GHOST },
		{ ELECTRIC, GROUND },
		{ FIGHTING, GHOST },
		{ POISON, STEEL },
		{ GROUND, FLYING },
		{ PSYCHIC, DARK },
		{ GHOST, NORMAL },
		{ DRAGON, FAIRY }
	};

	constexpr static eTypeEffectiveness CalculateEffectiveness(const monster_type attacker, const monster_type defender)
	{
		const eTypeFlags attackType = static_cast<eTypeFlags>(attacker);

		bool isImmune = false;
		bool isSuperEffective = false;
		bool isNotVeryEffective = false;

		for (unsigned i = 1; i <= NUM_TYPES; ++i)
		{
			const uint32_t bit = (1u << i);

			if ((defender & bit) == 0U)
			{
				continue;
			}

			if (IMMUNE.contains(attackType) && (IMMUNE.at(attackType) & bit))
			{
				isImmune = true;
				break;
			}

			if (SUPER_EFFECTIVE.contains(attackType) && (SUPER_EFFECTIVE.at(attackType)
				& bit))
			{
				isSuperEffective = true;
				continue;
			}

			if (NOT_VERY_EFFECTIVE.contains(attackType) && (NOT_VERY_EFFECTIVE.
				at(attackType) & bit))
			{
				isNotVeryEffective = true;
				continue;
			}
		}

		if (isImmune)
		{
			return eTypeEffectiveness::Immune;
		}
		if (isSuperEffective && isNotVeryEffective)
		{
			return eTypeEffectiveness::Effective;
		}
		if (isSuperEffective)
		{
			return eTypeEffectiveness::SuperEffective;
		}
		if (isNotVeryEffective)
		{
			return eTypeEffectiveness::NotVeryEffective;
		}
		return eTypeEffectiveness::Effective;
	}
}

namespace defending_maps
{
	constexpr frozen::unordered_map<eTypeFlags, monster_type, NUM_TYPES> RESISTANT =
	{
		{ NORMAL, 0u },
		{ FIRE, BUG | STEEL | ICE | FIRE | FAIRY | GRASS },
		{ WATER, WATER | FIRE | ICE | STEEL },
		{ GRASS, GRASS | WATER | GROUND | ELECTRIC },
		{ ELECTRIC, FLYING | STEEL | ELECTRIC },
		{ ICE, ICE },
		{ FIGHTING, BUG | DARK | ROCK },
		{ POISON, BUG | FIGHTING | POISON | FAIRY | GRASS },
		{ GROUND, POISON | ROCK },
		{ FLYING, BUG | FIGHTING | GRASS },
		{ PSYCHIC, FIGHTING | PSYCHIC },
		{ BUG, FIGHTING | GROUND | GRASS },
		{ ROCK, NORMAL | FLYING | FIRE | POISON },
		{ GHOST, BUG | POISON },
		{ DRAGON, ELECTRIC | FIRE | WATER | GRASS },
		{ DARK, DARK | GHOST },
		{ STEEL, FAIRY | STEEL | DRAGON | ROCK | BUG | PSYCHIC | FLYING | STEEL | ICE | GRASS | NORMAL },
		{ FAIRY, BUG | DARK | FIGHTING }
	};

	constexpr frozen::unordered_map<eTypeFlags, monster_type, NUM_TYPES> WEAK =
	{
		{ NORMAL, FIGHTING },
		{ FIRE, ROCK | GROUND | WATER },
		{ WATER, GRASS | ELECTRIC },
		{ GRASS, BUG | FLYING | ICE | FIRE | POISON },
		{ ELECTRIC, GROUND },
		{ ICE, FIGHTING | STEEL | FIRE | ROCK },
		{ FIGHTING, PSYCHIC | FLYING | FAIRY },
		{ POISON, PSYCHIC | GROUND },
		{ GROUND, ICE | WATER | GRASS },
		{ FLYING, ELECTRIC | ICE | ROCK },
		{ PSYCHIC, BUG | GHOST | DARK },
		{ BUG, FLYING | FIRE | ROCK },
		{ ROCK, FIGHTING | STEEL | GROUND | WATER | GRASS },
		{ GHOST, DARK | GHOST },
		{ DRAGON, DRAGON | ICE | FAIRY },
		{ DARK, FIGHTING | BUG | FAIRY },
		{ STEEL, GROUND | FIGHTING | FIRE },
		{ FAIRY, POISON | STEEL }
	};

	constexpr frozen::unordered_map<eTypeFlags, monster_type, 6> IMMUNE =
	{
		{ NORMAL, GHOST },
		{ GROUND, ELECTRIC },
		{ GHOST, NORMAL | FIGHTING },
		{ DARK, PSYCHIC },
		{ STEEL, POISON },
		{ FAIRY, DRAGON }
	};

	constexpr static eTypeEffectiveness CalculateEffectiveness(const monster_type defender, const monster_type attacker)
	{
		const eTypeFlags attackType = static_cast<eTypeFlags>(attacker);

		bool isImmune = false;
		bool isWeak = false;
		bool isResistant = false;

		for (int i = 1; i <= 18; ++i)
		{
			const uint32_t bit = (1u << i);
			if (!(defender & bit))
			{
				continue;
			}

			const eTypeFlags defType = static_cast<eTypeFlags>(bit);

			if (IMMUNE.contains(defType) && (IMMUNE.at(defType) & attackType))
			{
				isImmune = true;
				break;
			}

			if (WEAK.contains(defType) && (WEAK.at(defType) & attackType))
			{
				isWeak = true;
			}

			if (RESISTANT.contains(defType) && (RESISTANT.at(defType) & attackType))
			{
				isResistant = true;
			}
		}

		if (isImmune)
		{
			return eTypeEffectiveness::Immune;
		}
		if (isWeak && isResistant)
		{
			return eTypeEffectiveness::Effective;
		}
		if (isWeak)
		{
			return eTypeEffectiveness::SuperEffective;
		}
		if (isResistant)
		{
			return eTypeEffectiveness::NotVeryEffective;
		}

		return eTypeEffectiveness::Effective;
	}
}


constexpr eTypeEffectiveness CalculateEffectiveness(const monster_type attacker, const monster_type defender, const bool isAttacking)
{
	return isAttacking
		       ? attacking_maps::CalculateEffectiveness(attacker, defender)
		       : defending_maps::CalculateEffectiveness(defender, attacker);
}
