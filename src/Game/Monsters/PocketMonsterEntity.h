#ifndef POCKETMONSTERENTITY_H
#define POCKETMONSTERENTITY_H

#include "PocketMonster.h"
#include "../../Engine/Entity.h"
#include "../../Engine/Animation/AnimationComponent.h"


class PocketMonsterEntity : public Entity
{
public:
	PocketMonsterEntity(uint64_t monsterID, uint8_t level, EntityAnimationComponent::eAnimationName initialAnim);
	~PocketMonsterEntity() override = default;

	uint32_t GetMonsterID() const;

	hash_type GetNameStringID() const;

	MonsterStats& GetStats();
	uint8_t GetLevel() const;

private:
	PocketMonster m_monsterInfo;
	MonsterStats m_currentStats;

	uint8_t m_currentLevel;
	uint64_t m_EVs;
	uint64_t m_IVs;
};

#endif // POCKETMONSTERENTITY_H
