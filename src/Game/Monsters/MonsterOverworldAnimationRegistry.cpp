#include "MonsterOverworldAnimationRegistry.h"

#include "../../Engine/Asserts.h"
#include "../../Engine/Parsers/XML/XmlDocument.h"
#include "../../Engine/CodeGen/Resources.hpp"


MonsterOverworldAnimationRegistry* MonsterOverworldAnimationRegistry::Get()
{
	static std::shared_ptr<MonsterOverworldAnimationRegistry> inst = nullptr;

	if (inst == nullptr)
	{
		inst = Create();
	}

	return inst.get();
}

std::shared_ptr<AnimationDictionary> MonsterOverworldAnimationRegistry::GetAnimationDict(
	const uint32_t dexNum,
	const eMonsterOverworldState state
)
{
	static constexpr const char* walking_key = "monster_%03u_overworld_walking_animations";
	static constexpr const char* flying_key = "monster_%03u_overworld_flying_animations";
	static constexpr const char* swimming_key = "monster_%03u_overworld_swimming_animations";

	static constexpr size_t walking_str_len = std::string_view{ walking_key }.length();
	static constexpr size_t flying_str_len = std::string_view{ flying_key }.length();
	static constexpr size_t swimming_str_len = std::string_view{ swimming_key }.length();

	char str[43];

	switch (state)
	{
	case eMonsterOverworldState::Walking:
		snprintf(str, walking_str_len, walking_key, dexNum);
		break;
	case eMonsterOverworldState::Flying:
		snprintf(str, flying_str_len, flying_key, dexNum);
		break;
	case eMonsterOverworldState::Swimming:
		snprintf(str, swimming_str_len, swimming_key, dexNum);
		break;
	}

	const hash_type key = HASH(str);

	switch (state)
	{
	case eMonsterOverworldState::Flying:
		return m_flyAnimations.at(key);
	case eMonsterOverworldState::Swimming:
		return m_swimAnimations.at(key);
	default:
		return m_walkAnimations.at(key);
	}
}

bool MonsterOverworldAnimationRegistry::Init()
{
	printf("Loading eMonsterOverworldState::Walking animations...\n");
	auto walkingAnimations = LoadAnimationsFromXML(eMonsterOverworldState::Walking);
	ASSERT(walkingAnimations.has_value());
	if (!walkingAnimations.has_value())
	{
		return false;
	}

	printf("Loading eMonsterOverworldState::Flying animations...\n");
	auto flyingAnimations = LoadAnimationsFromXML(eMonsterOverworldState::Flying);
	ASSERT(flyingAnimations.has_value());
	if (!flyingAnimations.has_value())
	{
		return false;
	}

	printf("Loading eMonsterOverworldState::Swimming animations...\n");
	auto swimmingAnimations = LoadAnimationsFromXML(eMonsterOverworldState::Swimming);
	ASSERT(swimmingAnimations.has_value());
	if (!swimmingAnimations.has_value())
	{
		return false;
	}

	m_walkAnimations = std::move(walkingAnimations.value());
	m_flyAnimations = std::move(flyingAnimations.value());
	m_swimAnimations = std::move(swimmingAnimations.value());
	return true;
}

std::optional<std::unordered_map<hash_type, std::shared_ptr<AnimationDictionary>>>
MonsterOverworldAnimationRegistry::LoadAnimationsFromXML(
	const eMonsterOverworldState state)
{
	std::string_view animDictHash;
	switch (state)
	{
	case eMonsterOverworldState::Walking:
		animDictHash = ("MONSTER_WALKING_ANIMATIONS");
		break;
	case eMonsterOverworldState::Flying:
		animDictHash = ("MONSTER_FLYING_ANIMATIONS");
		break;
	case eMonsterOverworldState::Swimming:
		animDictHash = ("MONSTER_SWIMMING_ANIMATIONS");
		break;
	}

	std::unordered_map<hash_type, std::shared_ptr<AnimationDictionary>> animDicts;

	XmlDocument doc;
	doc.Load(GET_ANIMATION_PATH(animDictHash));

	const std::vector<const XmlNode*> dictionaries = doc.Root().Child("MonsterOverworldAnimations")->Children("AnimDict");
	size_t len = dictionaries.size();
	size_t registered = 0;
	for (const auto& dictionary : dictionaries)
	{
		auto dict = AnimationDictionary::Create(*dictionary);
		ASSERT(dict);
		if (!dict)
		{
			return std::nullopt;
		}

		printf("Loaded dict %llu/%llu\n", registered, len);

		animDicts[dict->GetName()] = dict;
		registered++;
	}

	return animDicts;
}
