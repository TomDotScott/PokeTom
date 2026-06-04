#include "Move.h"


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
           const std::optional<StatChange>& statChange)
	: m_id(id),
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
}
