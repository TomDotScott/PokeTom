#ifndef MONSTEROVERWORLDANIMATIONCOMPONENT_H
#define MONSTEROVERWORLDANIMATIONCOMPONENT_H
#include "MonsterOverworldAnimationRegistry.h"
#include "../../Engine/Animation/AnimationComponent.h"

class MonsterOverworldAnimationComponent : public EntityAnimationComponent
{
public:
	enum eAnimationName
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
	};

	MonsterOverworldAnimationComponent(Entity* owner, uint32_t dexNum, eMonsterOverworldState state, eAnimationName initialAnimation);

	// TODO: Fun little interactions with the player like in HGSS
	void OnPlayerInteractPressed() override;
	~MonsterOverworldAnimationComponent() override = default;

	static eAnimationName GetIdleAnimation(eMonsterOverworldState state, eOrientation orientation);
	static eAnimationName GetSprintAnimation(eMonsterOverworldState state, eOrientation orientation);
	static eAnimationName GetWalkAnimation(eMonsterOverworldState state, eOrientation orientation);
	static eAnimationName GetCycleAnimation(eMonsterOverworldState state, eOrientation orientation);

private:
	static hash_type GetAnimationName(eMonsterOverworldState state, eAnimationName name);
};

#endif // MONSTEROVERWORLDANIMATIONCOMPONENT_H
