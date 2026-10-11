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

class MonsterOverworldAnimationRegistry final : public Factory<MonsterOverworldAnimationRegistry>
{
public:
	friend class Factory;
	static MonsterOverworldAnimationRegistry* Get();

	std::shared_ptr<AnimationDictionary> GetAnimationDict(uint32_t dexNum, eMonsterOverworldState state);

protected:
	bool Init() override;

private:
	std::unordered_map<hash_type, std::shared_ptr<AnimationDictionary>> m_walkAnimations;
	std::unordered_map<hash_type, std::shared_ptr<AnimationDictionary>> m_flyAnimations;
	std::unordered_map<hash_type, std::shared_ptr<AnimationDictionary>> m_swimAnimations;

	std::optional<std::unordered_map<hash_type, std::shared_ptr<AnimationDictionary>>> LoadAnimationsFromXML(eMonsterOverworldState);
};

#endif // MONSTEROVERWORLDANIMATIONREGISTRY_H
