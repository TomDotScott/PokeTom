#ifndef MONSTERDATABASE_H
#define MONSTER_DATABASE_H
#include <filesystem>
#include <unordered_map>

#include "PocketMonster.h"
#include "../../Engine/Factory.h"

class PocketMonsterManager : public Factory<PocketMonsterManager>
{
public:
	friend class Factory;
	static PocketMonsterManager* Get();
	virtual ~PocketMonsterManager() = default;

	const PocketMonster& GetMonsterDetails(uint32_t monsterID) const;

protected:
	bool Init() override;

private:
	std::unordered_map<uint32_t, PocketMonster> m_monsterDetails;

	bool Load(const std::filesystem::path& path);
};

#endif
