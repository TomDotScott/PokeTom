#ifndef MONSTEROVERWORLDANIMATIONREGISTRY_H
#define MONSTEROVERWORLDANIMATIONREGISTRY_H

#include <unordered_map>

#include "../../Engine/Factory.h"
#include "../../Engine/Hash.h"
#include "../../Engine/Animation/AnimDef.h"

enum class eMonsterOverworldState
{
	Walking,
	Flying,
	Swimming
};

using overworld_animation_t = std::unordered_map<hash_type, std::shared_ptr<AnimationDictionary>>;

class MonsterOverworldAnimationRegistry final : public Factory<MonsterOverworldAnimationRegistry>
{
public:
	friend class Factory;
	static MonsterOverworldAnimationRegistry* Get();

	std::shared_ptr<AnimationDictionary> GetAnimationDict(uint32_t dexNum, eMonsterOverworldState state);

protected:
	bool Init() override;

private:
	overworld_animation_t m_walkAnimations;
	overworld_animation_t m_flyAnimations;
	overworld_animation_t m_swimAnimations;

	std::optional<overworld_animation_t> LoadAnimationsFromXML(eMonsterOverworldState);
};

#endif // MONSTEROVERWORLDANIMATIONREGISTRY_H
