#include "MonsterOverworldAnimationComponent.h"

#include "../../Engine/Asserts.h"


MonsterOverworldAnimationComponent::MonsterOverworldAnimationComponent(
	Entity* owner,
	const uint32_t dexNum,
	const eMonsterOverworldState state,
	const eAnimationName initialAnimation) :
	EntityAnimationComponent(
		owner,
		MonsterOverworldAnimationRegistry::Get()->GetAnimationDict(dexNum, state)
	)
{
	m_player.PlayAnimation(GetAnimationName(state, initialAnimation), true);
}

void MonsterOverworldAnimationComponent::OnPlayerInteractPressed()
{
	EntityAnimationComponent::OnPlayerInteractPressed();
}

hash_type MonsterOverworldAnimationComponent::GetAnimationName(const eMonsterOverworldState state, const eAnimationName name)
{
	switch (state)
	{
	case eMonsterOverworldState::Flying:
		{
			switch (name)
			{
			case UP:
				return HASH("flying_up");
			case DOWN:
				return HASH("flying_down");
			case LEFT:
				return HASH("flying_left");
			case RIGHT:
				return HASH("flying_right");
			}
			break;
		}
	case eMonsterOverworldState::Swimming:
		{
			switch (name)
			{
			case UP:
				return HASH("swim_up");
			case DOWN:
				return HASH("swim_down");
			case LEFT:
				return HASH("swim_left");
			case RIGHT:
				return HASH("swim_right");
			}
			break;
		}
	default:
		{
			switch (name)
			{
			case UP:
				return HASH("walk_up");
			case DOWN:
				return HASH("walk_down");
			case LEFT:
				return HASH("walk_left");
			case RIGHT:
				return HASH("walk_right");
			}
			break;
		}
	}

	ASSERT(false);
	return HASH("walk_up");
}
