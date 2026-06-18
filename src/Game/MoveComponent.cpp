#include "MoveComponent.h"

#include "MoveManager.h"
#include "../Engine/Asserts.h"
#include "../Engine/Entity.h"


MoveComponent::MoveComponent(Entity* owner, const std::array<uint32_t, MOVE_COUNT>& moves) :
	m_owner{ owner },
	m_moves{}
{
	for (size_t i = 0; i < moves.size(); ++i)
	{
		const unsigned moveID = moves[i];
		if (moveID != ~0U)
		{
			m_moves[i] = MoveManager::Get()->GetMove(moveID);
		}
	}
}

void MoveComponent::Update(float deltaTime)
{
}

bool MoveComponent::CanUseMove(const uint8_t moveIdx) const
{
	ASSERT(moveIdx < MOVE_COUNT);

	return m_moves[moveIdx].GetPPRemaining() > 0;
}

Move::Outcome MoveComponent::UseMove(const uint8_t moveIdx, Entity& target)
{
	ASSERT(m_owner != nullptr);

	if (CanUseMove(moveIdx))
	{
		return m_moves[moveIdx].Use(*m_owner, target);
	}

	ASSERT(false);
	return {
		.m_IsCriticalHit = false,
		.m_Damage = 0,
		.m_TypeMultiplier = 1.f
	};
}

Move& MoveComponent::GetMove(const uint8_t moveIdx)
{
	ASSERT(moveIdx < MOVE_COUNT);
	return m_moves[moveIdx];
}

const Move& MoveComponent::GetMove(const uint8_t moveIdx) const
{
	ASSERT(moveIdx < MOVE_COUNT);
	return m_moves[moveIdx];
}
