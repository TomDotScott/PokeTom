#include "PocketMonsterEntity.h"

#include "PocketMonsterManager.h"


namespace
{
	RandomRangeGenerator<uint64_t> SIXTY_FOUR_BIT_GENERATOR(0, 0xFFFFFFFFFFFFFFFF);
}


PocketMonsterEntity::PocketMonsterEntity(const uint64_t monsterID, const uint8_t level, EntityAnimationComponent::eAnimationName initialAnim) :
	m_monsterInfo(PocketMonsterManager::Get()->GetMonsterDetails(monsterID)),
	m_currentLevel(level)
{
	m_IVs = SIXTY_FOUR_BIT_GENERATOR.Next();
	m_EVs = SIXTY_FOUR_BIT_GENERATOR.Next();

	m_currentStats = MonsterStats::GetNextStat(m_monsterInfo.GetBaseStats(), m_currentLevel, m_IVs, m_EVs);

	std::string dictName = "MONSTER_" + std::to_string(monsterID) + "_BATTLE";

	AddComponent<EntityAnimationComponent>(this, dictName, initialAnim);
	SetScale(GetScale() * 2.f);
}

uint32_t PocketMonsterEntity::GetMonsterID() const
{
	return m_monsterInfo.GetID();
}

hash_type PocketMonsterEntity::GetNameStringID() const
{
	return m_monsterInfo.GetName();
}

MonsterStats& PocketMonsterEntity::GetStats()
{
	return m_currentStats;
}

uint8_t PocketMonsterEntity::GetLevel() const
{
	return m_currentLevel;
}
