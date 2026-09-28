#ifndef MONSTERPARTYCOMPONENT_H
#define MONSTERPARTYCOMPONENT_H

#include "PocketMonsterEntity.h"
#include "../../Engine/IUpdateable.h"

struct GameContext;

class MonsterPartyComponent : public IUpdateable
{
public:
	static constexpr uint8_t k_totalMonsters = 6;

	struct MonsterPartyInfo
	{
		PocketMonster m_Monster;
		unsigned m_Level;
	};

	MonsterPartyComponent(Entity* owner, GameContext& gameCtx, const std::vector<MonsterPartyInfo>& monsters);

	void Update(float deltaTime) override;

	void OnActivate() override;
	void OnDeactivate() override;

	void FullHeal() const;

	void OnBattleBegin();
	void OnBattleEnd() const;

	bool HasMonstersLeft() const;

	PocketMonsterEntity* GetActiveMonster() const;
	PocketMonsterEntity* SetActiveMonster(uint8_t monsterIdx);

private:
	// TODO: Need to come up with a nice way of passing this around
	GameContext& m_gameContext;

	uint8_t m_activeMonsterIdx;

	std::vector<MonsterPartyInfo> m_partyMonsters;
	std::array<PocketMonsterEntity*, k_totalMonsters> m_partyEntities;
};

#endif // MONSTERPARTYCOMPONENT_H
