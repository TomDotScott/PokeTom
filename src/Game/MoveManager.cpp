#include "MoveManager.h"

#include <fstream>
#include <iostream>
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

std::optional<MoveStatus> MoveManager::LoadStatusEffects(const nlohmann::json& move)
{
	const bool moveHasStatus = move.contains("status_inflicted");
	if (!moveHasStatus)
	{
		return std::nullopt;
	}

	std::vector<std::string> statusStrings;
	std::string token;
	std::istringstream isstream(move["status_inflicted"].get<std::string>());

	while (std::getline(isstream, token, ','))
	{
		const auto start = token.find_first_not_of(" \t\r\n");
		const auto end = token.find_last_not_of(" \t\r\n");
		if (start != std::string::npos)
		{
			statusStrings.push_back(token.substr(start, end - start + 1));
		}
	}

	unsigned status = 0U;
	for (const auto& ss : statusStrings)
	{
		const auto statusEnum = magic_enum::enum_cast<MoveStatus::eStatus>(
			ss,
			magic_enum::case_insensitive
		);

		ASSERT(statusEnum.has_value());
		if (statusEnum.has_value())
		{
			status |= statusEnum.value();
		}
		else
		{
			std::cout << "MoveManager::LoadStatusEffects - Unknown string! " << ss << "\n";
			return std::nullopt;
		}
	}

	uint8_t statusChance = 100;
	const bool moveHasStatusChance = move.contains("status_chance");
	if (moveHasStatusChance)
	{
		statusChance = move["status_chance"];
	}

	return MoveStatus{
		.m_Status = status,
		.m_Chance = statusChance
	};
}

std::optional<StatChange> MoveManager::LoadStatChange(const nlohmann::json& move)
{
	if (!move.contains("stat_changes"))
	{
		return std::nullopt;
	}


	const auto& statChangesJSON = move["stat_changes"];
	std::vector<StatChange::StatStage> statStages;
	statStages.reserve(statChangesJSON.size());

	for (const auto& statChange : statChangesJSON)
	{
		ASSERT(statChange.contains("stat") && statChange.contains("stages"));

		static std::unordered_map<std::string_view, MonsterStats::eStat> STAT_STRINGS{
			{ "hp", MonsterStats::eStat::HP },
			{ "attack", MonsterStats::eStat::Attack },
			{ "defense", MonsterStats::eStat::Defense },
			{ "special-attack", MonsterStats::eStat::SpecialAttack },
			{ "special-defense", MonsterStats::eStat::SpecialDefense },
			{ "speed", MonsterStats::eStat::Speed },
			{ "accuracy", MonsterStats::eStat::Accuracy },
			{ "evasion", MonsterStats::eStat::Evasion },
		};

		const MonsterStats::eStat stat = STAT_STRINGS.at(statChange["stat"]);
		const int stages = statChange["stages"];

		ASSERT(stages >= -6 && stages <= 6);

		statStages.emplace_back(stat, stages);
	}

	int chance = 100;
	if (move.contains("stat_change_chance"))
	{
		chance = move["stat_change_chance"];
	}

	StatChange::eChangeAffects affects = StatChange::eChangeAffects::Target;
	if (move.contains("stat_change_target"))
	{
		std::string target = move["stat_change_target"];
		if (target == "user")
		{
			affects = StatChange::eChangeAffects::User;
		}
		else if (target == "target")
		{
			affects = StatChange::eChangeAffects::Target;
		}
		else if (target == "both")
		{
			affects = StatChange::eChangeAffects::Both;
		}
		else
		{
			ASSERT_MSG(false, "Unknown stat_change_target value %s", target.c_str());
		}
	}

	return StatChange{
		.m_StatStages = statStages,
		.m_Chance = static_cast<uint8_t>(chance),
		.m_AffectedParty = affects
	};
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

		std::optional<unsigned> movePower = std::nullopt;
		const bool moveDoesDamage = move.contains("power");
		if (moveDoesDamage)
		{
			movePower = move["power"];
		}

		std::optional<uint8_t> moveAccuracy = std::nullopt;
		const bool moveHasAccuracy = move.contains("accuracy");
		if (moveHasAccuracy)
		{
			moveAccuracy = move["accuracy"];
		}

		std::optional<MoveStatus> moveStatus = LoadStatusEffects(move);
		std::optional<StatChange> statChange = LoadStatChange(move);

		movesMap.emplace(moveID, Move(moveID,
		                              idHash,
		                              descriptionHash,
		                              typeFlag,
		                              moveCategory,
		                              ppCount,
		                              movePriority,
		                              moveTarget,
		                              moveEffectCategory,
		                              movePower,
		                              moveAccuracy,
		                              moveStatus,
		                              statChange
		                 ));
	}

	m_moves = movesMap;

#if BUILD_DEBUG
	printf("MoveManager::LoadJSON - Loaded %llu moves\n", m_moves.size());
#endif

	f.close();
	return true;
}
