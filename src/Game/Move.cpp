#include "Move.h"

#include "../Engine/Asserts.h"
#include "../Engine/Entity.h"
#include "Monsters/PocketMonsterEntity.h"
#include <iostream>


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

void Move::Use(Entity& attacker, Entity& defender)
{
	ASSERT(m_valid);

	PocketMonsterEntity* attackerMonster = dynamic_cast<PocketMonsterEntity*>(&attacker);
	ASSERT(attackerMonster != nullptr);

	PocketMonsterEntity* defenderMonster = dynamic_cast<PocketMonsterEntity*>(&defender);
	ASSERT(defenderMonster != nullptr);

	monster_type attkType = defenderMonster->GetType();
	monster_type defType = defenderMonster->GetType();

	eTypeEffectiveness effectiveness = CalculateEffectiveness(attkType, defType, true);

	std::cout << attackerMonster->GetNameStringID() << " used " << m_stringTableID << " and it was " << magic_enum::enum_name(effectiveness);
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
