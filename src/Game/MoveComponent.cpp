#include "MoveComponent.h"

#include "MoveManager.h"
#include "../Engine/Asserts.h"
#include "../Engine/Entity.h"


MoveComponent::MoveComponent(Entity* owner, const std::array<uint32_t, 4>& moves) :
	m_owner{ owner },
	m_moves{ }
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
	ASSERT(moveIdx <= 3);

	return m_moves[moveIdx].GetPPRemaining() > 0;
}

bool MoveComponent::UseMove(const uint8_t moveIdx, Entity& target)
{
	ASSERT(m_owner != nullptr);

	if (CanUseMove(moveIdx))
	{
		m_moves[moveIdx].Use(*m_owner, target);
	}

	return false;
}
