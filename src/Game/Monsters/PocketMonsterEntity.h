#ifndef POCKETMONSTERENTITY_H
#define POCKETMONSTERENTITY_H

#include "PocketMonster.h"
#include "../../Engine/Entity.h"
#include "../../Engine/Animation/AnimationComponent.h"


class PocketMonsterEntity : public Entity
{
public:
	PocketMonsterEntity(uint32_t monsterID, uint8_t level, EntityAnimationComponent::eAnimationName initialAnim);
	~PocketMonsterEntity() override = default;

	uint32_t GetMonsterID() const;

	hash_type GetNameStringID() const;

	MonsterStats& GetStats();
	uint8_t GetLevel() const;

	monster_type GetType() const;

	std::string GetMoveName(uint8_t moveIdx);

private:
	PocketMonster m_monsterInfo;

	uint8_t m_currentLevel;
	std::array<uint8_t, STAT_COUNT> m_EVs;
	std::array<uint8_t, STAT_COUNT> m_IVs;
};

#endif // POCKETMONSTERENTITY_H
