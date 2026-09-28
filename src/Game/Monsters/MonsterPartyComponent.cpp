#include "MonsterPartyComponent.h"

#include "../GameContext.h"
#include "../../Engine/Asserts.h"


MonsterPartyComponent::MonsterPartyComponent(Entity* owner, GameContext& gameCtx,
                                             const std::vector<MonsterPartyInfo>& monsters) :
	m_gameContext(gameCtx),
	m_activeMonsterIdx(0),
	m_partyMonsters(monsters),
	m_partyEntities()
{
	ASSERT(!monsters.empty() && monsters.size()<= k_totalMonsters);
}

void MonsterPartyComponent::Update(const float /*deltaTime*/)
{
}

void MonsterPartyComponent::OnActivate()
{
	IUpdateable::OnActivate();

	size_t i = 0;
	for (; i < m_partyMonsters.size(); ++i)
	{
		auto& [monsterInfo, monsterLevel] = m_partyMonsters[i];

		auto& entity = m_gameContext.m_Entities.Create<PocketMonsterEntity>(
			monsterInfo.GetID(),
			monsterLevel,
			EntityAnimationComponent::eAnimationName::BATTLE_BACK
		);

		entity.OnDeactivate();

		// Hopefully this is safe?
		m_partyEntities[i] = &entity;
	}

	if (i < k_totalMonsters)
	{
		for (; i < k_totalMonsters; ++i)
		{
			m_partyEntities[i] = nullptr;
		}
	}
}

void MonsterPartyComponent::OnDeactivate()
{
	IUpdateable::OnDeactivate();

	for (auto& entity : m_partyEntities)
	{
		if (entity == nullptr)
		{
			continue;
		}

		m_gameContext.m_Entities.Destroy(entity->GetID());

		entity = nullptr;
	}
}

void MonsterPartyComponent::FullHeal() const
{
	for (auto monsterEntity : m_partyEntities)
	{
		if (monsterEntity != nullptr)
		{
			monsterEntity->FullHeal();
		}
	}
}

void MonsterPartyComponent::OnBattleBegin()
{
	// Find the first monster with HP and set that to active
	for (size_t i = 0; i < m_partyEntities.size(); ++i)
	{
		if (m_partyEntities[i] == nullptr)
		{
			continue;
		}

		if (!m_partyEntities[i]->IsFainted())
		{
			m_activeMonsterIdx = static_cast<uint8_t>(i);
			break;
		}
	}

	GetActiveMonster()->OnActivate();
}

void MonsterPartyComponent::OnBattleEnd() const
{
	for (auto& entity : m_partyEntities)
	{
		if (entity == nullptr)
		{
			continue;
		}

		entity->OnDeactivate();
	}
}

bool MonsterPartyComponent::HasMonstersLeft() const
{
	return std::ranges::any_of(m_partyEntities.begin(), m_partyEntities.end(), [&](const PocketMonsterEntity* monster)
	{
		return monster != nullptr && !monster->IsFainted();
	});
}

PocketMonsterEntity* MonsterPartyComponent::GetActiveMonster() const
{
	return m_partyEntities[m_activeMonsterIdx];
}

PocketMonsterEntity* MonsterPartyComponent::SetActiveMonster(const uint8_t monsterIdx)
{
	ASSERT(monsterIdx <= k_totalMonsters);
	m_activeMonsterIdx = monsterIdx;

	return GetActiveMonster();
}
