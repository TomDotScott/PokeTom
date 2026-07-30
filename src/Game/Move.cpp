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

Move::Outcome Move::Use(Entity& attacker, Entity& defender)
{
	ASSERT(m_valid);

	PocketMonsterEntity* attackerMonster = dynamic_cast<PocketMonsterEntity*>(&attacker);
	ASSERT(attackerMonster != nullptr);

	PocketMonsterEntity* defenderMonster = dynamic_cast<PocketMonsterEntity*>(&defender);
	ASSERT(defenderMonster != nullptr);

	const monster_type attkType = m_type;
	const monster_type defType = defenderMonster->GetType();

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

	uint16_t damageDealt = 0;
	const bool isCriticalHit = criticalRNG.Next() == 1;

	// https://www.bulbapedia.bulbagarden.net/wiki/Damage
	const float typeMultiplier = CalculateAttackingEffectiveness(attkType, defType);
	if (typeMultiplier == 0.f)
	{
		return {
			.m_MoveMissed = false,
			.m_IsCriticalHit = false,
			.m_Damage = 0,
			.m_TypeMultiplier = typeMultiplier
		};
	}

	if (m_category != eMoveCategory::Status && m_power.has_value())
	{
		const float levelModifier = 2.f * static_cast<float>(attackerMonster->GetLevel()) / 5.f + 2.f;

		float attackStat = 1.f;
		float defenseStat = 1.f;

		if (m_category == eMoveCategory::Physical)
		{
			attackStat = attackerMonster->GetStats().m_Attack;
			defenseStat = defenderMonster->GetStats().m_Defense;
		}
		else if (m_category == eMoveCategory::Special)
		{
			attackStat = attackerMonster->GetStats().m_SpAttack;
			defenseStat = defenderMonster->GetStats().m_SpDefense;
		}

		const float aOverD = attackStat / defenseStat;

		float firstPart = levelModifier * static_cast<float>(m_power.value()) * aOverD / 50.f + 2.f;

		const float targets = 1.f;
		const float pb = 1.f;

		// TODO:
		const float weather = 1.f;

		const float critical = isCriticalHit ? 1.5f : 1.0f;
		const float random = static_cast<float>(moveRNG.Next()) / 100.f;
		const float stab = m_type & attkType ? 1.5f : 1.0f;

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

		defenderMonster->TakeDamage(damageDealt);
	}

	std::optional<Outcome::StatChangeOutcome> changedStat = std::nullopt;
	if (m_statChange.has_value())
	{
		StatChange change = m_statChange.value();
		const uint16_t rng = percentageRNG.Next();
		const bool success = change.m_Chance > rng;
		if (success)
		{
			bool affectedDefender = false;
			bool affectedAttacker = false;

			switch (m_target)
			{
			case eMoveTarget::AllPokemon:
			case eMoveTarget::EntireField:
				affectedDefender = true;
				affectedAttacker = true;
				break;
			case eMoveTarget::Selected:
			case eMoveTarget::AllOpponents:
			case eMoveTarget::AllOther:
			case eMoveTarget::OpponentField:
				affectedDefender = true;
				break;
			case eMoveTarget::RandomOpponent:
				// TODO:
				break;
			case eMoveTarget::SpecificMove:
				// TODO:
				break;
			case eMoveTarget::Ally:
			case eMoveTarget::UserAndAllies:
			case eMoveTarget::UserOrAlly:
			case eMoveTarget::UserField:
			case eMoveTarget::User:
				affectedAttacker = true;
				break;
			case eMoveTarget::Unknown:
				ASSERT(false);
				break;
			}

			std::vector<StatChange::StatStage> attackerChangedStats;
			std::vector<StatChange::StatStage> defenderChangedStats;
			attackerChangedStats.reserve(change.m_StatStages.size());
			defenderChangedStats.reserve(change.m_StatStages.size());

			for (const StatChange::StatStage& stat : change.m_StatStages)
			{
				if (affectedAttacker)
				{
					attackerMonster->ModifyStat(stat.m_Stat, stat.m_Stages);
					attackerChangedStats.emplace_back(stat);
				}

				if (affectedDefender)
				{
					defenderMonster->ModifyStat(stat.m_Stat, stat.m_Stages);
					defenderChangedStats.emplace_back(stat);
				}
			}

			changedStat = Outcome::StatChangeOutcome{
				.m_AttackerStatChanges = attackerChangedStats,
				.m_DefenderStatChanges = defenderChangedStats
			};
		}
	}


	m_powerPoints--;
	return {
		.m_MoveMissed = false,
		.m_IsCriticalHit = isCriticalHit,
		.m_Damage = damageDealt,
		.m_TypeMultiplier = typeMultiplier,
		.m_StatChangeOutcome = changedStat
	};
}

unsigned Move::GetPPRemaining() const
{
	return m_powerPoints;
}

bool Move::IsValid() const
{
	return m_valid;
}

hash_type Move::GetNameStringTableID() const
{
	return m_stringTableID;
}
