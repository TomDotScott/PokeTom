#include "MoveManager.h"

#include <fstream>
#include <json.hpp>

#include "../Engine/Asserts.h"


MoveManager* MoveManager::Get()
{
	static std::shared_ptr<MoveManager> mm = nullptr;

	if (mm == nullptr)
	{
		mm = MoveManager::Create();
	}

	return mm.get();
}

const Move& MoveManager::GetMove(const uint32_t moveID) const
{
	ASSERT(m_moves.contains(moveID));
	return m_moves.at(moveID);
}

bool MoveManager::Init()
{
	return LoadJSON();
}

bool MoveManager::LoadJSON()
{
	std::ifstream f;
	f.open("moves.json");

	nlohmann::json root = nlohmann::json::parse(f);

	std::unordered_map<uint32_t, Move> movesMap;

	const auto& movesArray = root["moves"];
	for (const auto& move : movesArray)
	{
		const auto& id = move["id"];
		uint32_t moveID = id;

		const auto& stringTableID = move["name"];
		hash_type idHash = HASH(stringTableID);

		const auto& descriptionStringTableID = move["effect_description"];
		hash_type descriptionHash = HASH(descriptionStringTableID);

		const auto& type = move["type"];
		const eTypeFlags typeFlag = magic_enum::enum_cast<eTypeFlags>(
			type.get<std::string>(),
			magic_enum::case_insensitive
		).value();

		const auto& category = move["category"];
		const eMoveCategory moveCategory = magic_enum::enum_cast<eMoveCategory>(
			category.get<std::string>(),
			magic_enum::case_insensitive
		).value();

		const auto& pp = move["pp"];
		const unsigned ppCount = pp;

		const auto& priority = move["priority"];
		const int movePriority = priority;

		const auto& target = move["target"];
		const eMoveTarget moveTarget = magic_enum::enum_cast<eMoveTarget>(
			target.get<std::string>(),
			magic_enum::case_insensitive
		).value();

		const auto& effectCategory = move["effect_category"];
		const eEffectCategory moveEffectCategory = magic_enum::enum_cast<eEffectCategory>(
			effectCategory.get<std::string>(),
			magic_enum::case_insensitive
		).value();

		movesMap.emplace(id, Move(id, stringTableID, descriptionStringTableID, typeFlag, moveCategory, ppCount, movePriority, moveTarget, moveEffectCategory, std::nullopt, std::nullopt, std::nullopt, std::nullopt));
	}

	m_moves = movesMap;

	f.close();

	return true;
}
