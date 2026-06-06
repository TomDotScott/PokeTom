#include "PocketMonsterEntity.h"

#include "PocketMonsterManager.h"
#include "../MoveComponent.h"
#include "../MoveManager.h"
#include "../../Engine/Asserts.h"


namespace
{
	RandomRangeGenerator<short> SIXTEEN_BIT_GENERATOR(0, 0xFF);
	RandomRangeGenerator<uint64_t> SIXTY_FOUR_BIT_GENERATOR(0, 0xFFFFFFFFFFFFFFFF);
}


PocketMonsterEntity::PocketMonsterEntity(const uint32_t monsterID, const uint8_t level,
										 EntityAnimationComponent::eAnimationName initialAnim) :
	m_monsterInfo(PocketMonsterManager::Get()->GetMonsterDetails(monsterID)),
	m_currentLevel(level)
{
	const auto& move = MoveManager::Get()->GetMove(1);

	m_IVs = {
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
	};

	m_EVs = {
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
		static_cast<uint8_t>(SIXTEEN_BIT_GENERATOR.Next()),
	};

	AddComponent<MonsterStatComponent>(MonsterStats::GetNextStat(m_monsterInfo.GetBaseStats(), m_currentLevel, m_IVs, m_EVs));

	AddComponent<MoveComponent>(this, std::array<uint32_t, 4>{ 33, 45, ~0U, ~0U});


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
	auto* msc = GetComponent<MonsterStatComponent>();
	ASSERT(msc != nullptr);

	return msc->GetStats();
}

uint8_t PocketMonsterEntity::GetLevel() const
{
	return m_currentLevel;
}

monster_type PocketMonsterEntity::GetType() const
{
	return m_monsterInfo.GetType();
}
