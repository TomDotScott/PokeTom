#include "PocketMonsterManager.h"

#include <fstream>
#include <iostream>
#include <json.hpp>
#include <magic_enum.hpp>

#include "MonsterTypes.h"
#include "../../Engine/Asserts.h"
#include "../../Engine/Hash.h"

PocketMonsterManager* PocketMonsterManager::Get()
{
	static std::shared_ptr<PocketMonsterManager> inst = nullptr;

	if (inst == nullptr)
	{
		inst = Create();
	}

	return inst.get();
}

const PocketMonster& PocketMonsterManager::GetMonsterDetails(const uint32_t monsterID) const
{
	ASSERT(m_monsterDetails.contains(monsterID));

	return m_monsterDetails.at(monsterID);
}

bool PocketMonsterManager::Init()
{
	return Load("pokedex.json");
}

bool PocketMonsterManager::Load(const std::filesystem::path& path)
{
	if (!std::filesystem::exists(path))
	{
		return false;
	}

	std::ifstream f(path);

	ASSERT(f.is_open());
	if (!f.is_open())
	{
		std::cout << "Error: Failed to open pokedex file!\n";
		return false;
	}

	nlohmann::json json = nlohmann::json::parse(f);

	auto monstersArray = json["pokemon"];
	if (monstersArray.type() != nlohmann::detail::value_t::array)
	{
		return false;
	}

	std::unordered_map<uint32_t, PocketMonster> monsters;

	for (const auto& elem : monstersArray)
	{
		int id = elem["id"];
		hash_type nameStringTableID = HASH(elem["name"]);

		monster_type type = 0u;
		const auto& typesArray = elem["type"];
		ASSERT(typesArray.is_array());

		for (const auto& typeString : typesArray)
		{
			// case-insensitive string to enum cast
			auto flag = magic_enum::enum_cast<eTypeFlags>(typeString.get<std::string>(), magic_enum::case_insensitive);

			if (flag.has_value())
			{
				type |= flag.value();
			}
		}

		evolution_tree nextEvolutions = DEFAULT_EVOLUTION_TREE;
		if (elem.find("next_evolution") != elem.end())
		{
			const auto& nextEvoArray = elem["next_evolution"];
			if (!nextEvoArray.is_null())
			{
				ASSERT(nextEvoArray.is_array());

				for (int i = 0; i < nextEvoArray.size(); ++i)
				{
					nextEvolutions[i] = nextEvoArray[i];
				}
			}
		}

		evolution_tree previousEvolutions = DEFAULT_EVOLUTION_TREE;
		if (elem.find("prev_evolution") != elem.end())
		{
			const auto& prevEvoArray = elem["prev_evolution"];
			if (!prevEvoArray.is_null())
			{
				ASSERT(prevEvoArray.is_array());

				for (int i = 0; i < prevEvoArray.size(); ++i)
				{
					previousEvolutions[i] = prevEvoArray[i];
				}
			}
		}

		const auto& stats = elem["stats"];
		ASSERT(!stats.is_null());
		if (stats.is_null())
		{
			return false;
		}

		const auto& hp = stats["hp"];
		ASSERT(hp.is_number());

		const auto& attack = stats["attack"];
		ASSERT(attack.is_number());

		const auto& defense = stats["defense"];
		ASSERT(defense.is_number());

		const auto& sp_atk = stats["sp_atk"];
		ASSERT(sp_atk.is_number());

		const auto& sp_def = stats["sp_def"];
		ASSERT(sp_def.is_number());

		const auto& speed = stats["speed"];
		ASSERT(speed.is_number());

		monsters.emplace(id, PocketMonster(
			id,
			type,
			MonsterStats(
				hp,
				attack,
				defense,
				sp_atk,
				sp_def,
				speed
			),
			previousEvolutions,
			nextEvolutions
		));
	}


	m_monsterDetails = monsters;

#if BUILD_DEBUG
	std::cout << "PocketMonsterManager::Load - Loaded pokedex with " << monsters.size() << " entries\n";
#endif

	return true;
}
