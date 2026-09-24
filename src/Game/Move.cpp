#include "Move.h"

#include "../Engine/Asserts.h"
#include "../Engine/Entity.h"
#include "Monsters/PocketMonsterEntity.h"

namespace
{
	// Is the move critical? According to u/AxeVice it is a 1/24 chance!
	RandomRangeGenerator<uint16_t> criticalRNG = RandomRangeGenerator<uint16_t>(1, 24);
	RandomRangeGenerator<uint16_t> moveRNG = RandomRangeGenerator<uint16_t>(85, 100);
	RandomRangeGenerator<uint16_t> percentageRNG = RandomRangeGenerator<uint16_t>(0, 100);
}


Move::Move(const uint32_t id,
           hash_type stringTableID,
           hash_type descriptionStringTableID,
           const monster_type type,
           const eMoveCategory category,
           const unsigned powerPoints,
           const int priority,
           const eMoveTarget target,
           const eEffectCategory effectCategory,
           const std::optional<unsigned int>& power,
           const std::optional<uint8_t>& accuracy,
           const std::optional<MoveStatus>& status,
           const std::optional<StatChange>& statChange) :
	m_valid{ true },
	m_id(id),
	m_stringTableID(stringTableID),
	m_descriptionStringTableID(descriptionStringTableID),
	m_type(type),
	m_category(category),
	m_powerPoints(powerPoints),
	m_maxPowerPoints(powerPoints),
	m_priority(priority),
	m_target(target),
	m_effectCategory(effectCategory),
	m_power(power),
	m_accuracy(accuracy),
	m_status(status),
	m_statChange(statChange)
{
	ASSERT(m_valid);
}

Move::Outcome Move::Use(PocketMonsterEntity& attacker, PocketMonsterEntity& defender)
{
	ASSERT(m_valid);

	const monster_type defType = defender.GetType();

	// TODO: Calculate this properly - for now, just do RNG based on the numbers!
	if (m_accuracy.has_value())
	{
		if (m_accuracy.value() < percentageRNG.Next())
		{
			return {
				.m_MoveMissed = true,
				.m_IsCriticalHit = false,
				.m_Damage = 0,
				.m_TypeMultiplier = 0.f
			};
		}
	}

	const bool isCriticalHit = criticalRNG.Next() == 1;

	// https://www.bulbapedia.bulbagarden.net/wiki/Damage
	const float typeMultiplier = CalculateAttackingEffectiveness(m_type, defType);
	if (typeMultiplier == 0.f)
	{
		return {
			.m_MoveMissed = false,
			.m_IsCriticalHit = false,
			.m_Damage = 0,
			.m_TypeMultiplier = typeMultiplier
		};
	}

	const uint16_t damageDealt = ApplyDamage(attacker, defender, typeMultiplier, isCriticalHit);
	const std::optional<Outcome::StatChangeOutcome> changedStat = ApplyStatChange(attacker, defender);
	const std::optional<Outcome::StatusEffect> statusEffects = ApplyStatusEffect(attacker, defender);

	m_powerPoints--;
	return {
		.m_MoveMissed = false,
		.m_IsCriticalHit = isCriticalHit,
		.m_Damage = damageDealt,
		.m_TypeMultiplier = typeMultiplier,
		.m_StatChangeOutcome = changedStat,
		.m_StatusEffect = statusEffects
	};
}

unsigned Move::GetPPRemaining() const
{
	return m_powerPoints;
}

unsigned Move::GetMaxPP() const
{
	return m_maxPowerPoints;
}

uint32_t Move::GetID() const
{
	return m_id;
}

monster_type Move::GetType() const
{
	return m_type;
}

bool Move::IsValid() const
{
	return m_valid;
}

hash_type Move::GetNameStringTableID() const
{
	return m_stringTableID;
}

hash_type Move::GetDescriptionStringTableID() const
{
	return m_descriptionStringTableID;
}

uint16_t Move::ApplyDamage(PocketMonsterEntity& attacker,
                           PocketMonsterEntity& defender,
                           const float typeMultiplier,
                           const bool isCriticalHit
) const
{
	uint16_t damageDealt = 0;

	if (m_category != eMoveCategory::Status && m_power.has_value())
	{
		const float levelModifier = 2.f * static_cast<float>(attacker.GetLevel()) / 5.f + 2.f;

		float attackStat = 1.f;
		float defenseStat = 1.f;

		if (m_category == eMoveCategory::Physical)
		{
			attackStat = attacker.GetStats().m_Attack;
			defenseStat = defender.GetStats().m_Defense;
		}
		else if (m_category == eMoveCategory::Special)
		{
			attackStat = attacker.GetStats().m_SpAttack;
			defenseStat = defender.GetStats().m_SpDefense;
		}

		const float aOverD = attackStat / defenseStat;

		const float firstPart = levelModifier * static_cast<float>(m_power.value()) * aOverD / 50.f + 2.f;

		const float targets = 1.f;
		const float pb = 1.f;

		// TODO:
		const float weather = 1.f;

		const float critical = isCriticalHit ? 1.5f : 1.0f;
		const float random = static_cast<float>(moveRNG.Next()) / 100.f;
		const float stab = m_type & attacker.GetType() ? 1.5f : 1.0f;

		// TODO:
		const float burn = 1.f;

		// TODO:
		const float other = 1.f;

		float damage = firstPart * targets * pb * weather * critical * random * stab * typeMultiplier * burn *
			other;

		// Round to the nearest integer. If 0, the value is 1.
		damage = std::round(damage);

		if (damage <= 0.f)
		{
			damage = 1.f;
		}

		damageDealt = static_cast<uint16_t>(damage);

#if BUILD_DEBUG
		printf("DEALING %u HP of DAMAGE\n", damageDealt);
#endif

		switch (m_target)
		{
		// TODO: Might be super effective against the user!
		case eMoveTarget::AllPokemon:
			break;
		case eMoveTarget::EntireField:
		case eMoveTarget::AllOther:
		case eMoveTarget::AllOpponents:
		case eMoveTarget::OpponentField:
		case eMoveTarget::RandomOpponent:
		case eMoveTarget::Selected:
			defender.TakeDamage(damageDealt);
			break;
		case eMoveTarget::Ally:
			// TODO: Needed?
			break;
		case eMoveTarget::SpecificMove:
			// TODO:
			break;
		case eMoveTarget::UserField:
		case eMoveTarget::UserOrAlly:
		case eMoveTarget::UserAndAllies:
		case eMoveTarget::User:
			attacker.TakeDamage(damageDealt);
			break;
		case eMoveTarget::Unknown:
			ASSERT(false);
			break;
		}
	}

	return damageDealt;
}

std::optional<Move::Outcome::StatChangeOutcome> Move::ApplyStatChange(PocketMonsterEntity& attacker,
                                                                      PocketMonsterEntity& defender) const
{
	if (!m_statChange.has_value())
	{
		return std::nullopt;
	}

	const auto [statStages, chance, affectedParty] = m_statChange.value();
	const uint16_t rng = percentageRNG.Next();

	if (chance < rng)
	{
		return std::nullopt;
	}

	const bool affectsBoth = affectedParty == StatChange::eChangeAffects::Both;
	const bool affectsAttacker = affectsBoth || affectedParty == StatChange::eChangeAffects::User;
	const bool affectsDefender = affectsBoth || affectedParty == StatChange::eChangeAffects::Target;

	std::vector<stat_change_outcome> attackerChangedStats;
	std::vector<stat_change_outcome> defenderChangedStats;
	attackerChangedStats.reserve(statStages.size());
	defenderChangedStats.reserve(statStages.size());

	for (const StatChange::StatStage& stat : statStages)
	{
		if (affectsAttacker)
		{
			attackerChangedStats.emplace_back(
				stat,
				attacker.ModifyStat(stat.m_Stat, stat.m_Stages)
			);
		}

		if (affectsDefender)
		{
			defenderChangedStats.emplace_back(
				stat,
				defender.ModifyStat(stat.m_Stat, stat.m_Stages)
			);
		}
	}

	return Outcome::StatChangeOutcome{
		.m_AttackerStatChanges = attackerChangedStats,
		.m_DefenderStatChanges = defenderChangedStats
	};
}

std::optional<Move::Outcome::StatusEffect> Move::ApplyStatusEffect(PocketMonsterEntity& attacker,
                                                                   PocketMonsterEntity& defender) const
{
	if (!m_status.has_value())
	{
		return std::nullopt;
	}

	const uint32_t statusEffects = m_status.value().m_StatusEffects;
	const uint16_t rng = percentageRNG.Next();

	printf("Status RNG %d\n", rng);

	const bool success = m_status.value().m_Chance >= rng;
	if (!success)
	{
		return std::nullopt;
	}

	PocketMonsterEntity& targetEntity = m_target == eMoveTarget::User ? attacker : defender;

	uint32_t appliedStatuses = NoStatus;
	// Go through the statuses and determine if any of them can be applied
	for (unsigned i = 0; i < NUM_STATUS_EFFECTS; ++i)
	{
		const uint32_t bit = (1u << i);

		if ((statusEffects & bit) == 0U)
		{
			continue;
		}

		if (targetEntity.ApplyStatusEffect(static_cast<eStatusEffect>(bit)))
		{
			appliedStatuses |= bit;
		}
	}

	return Move::Outcome::StatusEffect{
		.m_AffectsDefender = m_target != eMoveTarget::User,
		.m_Flags = appliedStatuses
	};
}
