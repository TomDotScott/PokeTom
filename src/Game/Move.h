#ifndef MOVE_H
#define MOVE_H
#include "../Engine/Hash.h"
#include "Monsters/MonsterStats.h"
#include "Monsters/MonsterTypes.h"

enum class eMoveCategory : uint8_t
{
	Physical,
	Special,
	Status
};

enum class eEffectCategory : uint8_t
{
	Damage,
	Status,
	StatChange,
	Healing,
	Drain,
	DamageStatus,
	DamageStatChange,
	OneHitKO,
	Field,
	ForceSwitch,
	Unique
};

enum class eMoveTarget : uint8_t
{
	AllPokemon,
	AllOpponents,
	AllOther,
	Ally,
	EntireField,
	OpponentField,
	RandomOpponent,
	Selected,
	SpecificMove,
	User,
	UserAndAllies,
	UserOrAlly,
	UserField,
	Unknown
};

struct MoveStatus
{
	enum eStatus
	{
		BURN = 1 << 0,
		FREEZE = 1 << 1,
		PARALYSIS = 1 << 2,
		POISON = 1 << 3,
		SLEEP = 1 << 4,
		CONFUSION = 1 << 5,
		INFATUATION = 1 << 6,
		TRAP = 1 << 7,
		DISABLE = 1 << 8,
		EMBARGO = 1 << 9,
		HEAL_BLOCK = 1 << 10,
		INGRAIN = 1 << 11,
		LEECH_SEED = 1 << 12,
		NIGHTMARE = 1 << 13,
		NO_TYPE_IMMUNITY = 1 << 14,
		PERISH_SONG = 1 << 15,
		SILENCE = 1 << 16,
		TAR_SHOT = 1 << 17,
		TORMENT = 1 << 18,
		YAWN = 1 << 19,
	} m_Status;

	unsigned char m_Chance;
};

template <>
struct magic_enum::customize::enum_range<MoveStatus::eStatus>
{
	static constexpr bool is_flags = true;
};

struct StatChange
{
	MonsterStats::eStat m_Stat;
	int m_Stages;
	uint8_t m_Chance;
};

class Move
{
public:
	Move(uint32_t id,
	     hash_type stringTableID,
	     hash_type descriptionStringTableID,
	     monster_type type,
	     eMoveCategory category,
	     unsigned powerPoints,
	     int priority,
	     eMoveTarget target,
	     eEffectCategory effectCategory,
	     const std::optional<unsigned>& power,
	     const std::optional<uint8_t>& accuracy,
	     const std::optional<MoveStatus>& status,
	     const std::optional<StatChange>& statChange
	);

private:
	uint32_t m_id;
	hash_type m_stringTableID;
	hash_type m_descriptionStringTableID;
	monster_type m_type;
	eMoveCategory m_category;
	unsigned m_powerPoints;
	int m_priority;
	eMoveTarget m_target;
	eEffectCategory m_effectCategory;

	std::optional<unsigned> m_power;
	std::optional<uint8_t> m_accuracy; // 0-100
	std::optional<MoveStatus> m_status;
	std::optional<StatChange> m_statChange;
};


#endif // MOVE_H
