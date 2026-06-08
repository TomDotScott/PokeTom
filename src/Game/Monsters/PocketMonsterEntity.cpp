#include "PocketMonsterEntity.h"

#include "PocketMonsterManager.h"
#include "../MoveComponent.h"
#include "../MoveManager.h"
#include "../../Engine/Asserts.h"
#include "../../Engine/Stringtable.h"


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

	AddComponent<MonsterStatComponent>(
		MonsterStats::GetNextStat(m_monsterInfo.GetBaseStats(), m_currentLevel, m_IVs, m_EVs));

	AddComponent<MoveComponent>(this, std::array<uint32_t, 4>{ 33, 45, 2, ~0U });

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

std::string PocketMonsterEntity::GetMoveName(const uint8_t moveIdx) const
{
	ASSERT(moveIdx < MOVE_COUNT);
	auto* moveComponent = GetComponent<MoveComponent>();
	ASSERT(moveComponent != nullptr);

	const Move& move = moveComponent->GetMove(moveIdx);
	ASSERT(move.IsValid());

	return STRINGTABLE->GetString(HASH("MOVE"), move.GetNameStringTableID());
}

bool PocketMonsterEntity::IsFainted() const
{
	const MonsterStatComponent* msc = GetComponent<MonsterStatComponent>();
	ASSERT(msc != nullptr);
	return msc->IsFainted();
}
