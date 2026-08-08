#ifndef POCKETMONSTERENTITY_H
#define POCKETMONSTERENTITY_H

#include "MonsterXP.h"
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

	MonsterStats GetStats();
	const MonsterStats& GetBaseStats() const;

	bool ModifyStat(MonsterStats::eStat stat, int stages);

	uint8_t GetLevel() const;
	monster_hp_t GetMaxHP() const;

	monster_type GetType() const;

	monster_xp_t GetCurrentXP() const;
	monster_xp::eGroup GetExperienceGroup() const;
	monster_xp_t GetExperienceYield() const;

	std::string GetMoveName(uint8_t moveIdx) const;


	bool IsFainted() const;
	void TakeDamage(monster_hp_t damage);
	void FullHeal();

	// Ideally, I think these should be joined up - but I can't think of a way
	// to show the level ups individually to the player without them
	// being separated at the moment...
	void GainExperience(monster_xp_t xp);
	void ApplyLevelUpStats(uint8_t newLevel);

	void OnDeactivate() override;

private:
	PocketMonster m_monsterInfo;

	monster_xp_t m_currentXP;
	std::array<uint8_t, STAT_COUNT> m_EVs;
	std::array<uint8_t, STAT_COUNT> m_IVs;
};

#endif // POCKETMONSTERENTITY_H
