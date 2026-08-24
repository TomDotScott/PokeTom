#ifndef MOVE_H
#define MOVE_H
#include "../Engine/Hash.h"
#include "Monsters/MonsterStats.h"
#include "Monsters/MonsterTypes.h"

class Entity;

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
		Burn = 1 << 0,
		Freeze = 1 << 1,
		Paralysis = 1 << 2,
		Poison = 1 << 3,
		Sleep = 1 << 4,
		Confusion = 1 << 5,
		Infatuation = 1 << 6,
		Trap = 1 << 7,
		Disable = 1 << 8,
		Embargo = 1 << 9,
		HealBlock = 1 << 10,
		Ingrain = 1 << 11,
		LeechSeed = 1 << 12,
		Nightmare = 1 << 13,
		NoTypeImmunity = 1 << 14,
		PerishSong = 1 << 15,
		Silence = 1 << 16,
		Torment = 1 << 17,
		Yawn = 1 << 18,
	};

	// Mask of eStatus flags
	uint32_t m_Status;

	unsigned char m_Chance;
};

template <>
struct magic_enum::customize::enum_range<MoveStatus::eStatus>
{
	static constexpr bool is_flags = true;
};

struct StatChange
{
	enum class eChangeAffects
	{
		User,
		Target,
		Both
	};

	struct StatStage
	{
		explicit StatStage(const MonsterStats::eStat stat, const int stages) :
			m_Stat(stat),
			m_Stages(stages)
		{
		}

		MonsterStats::eStat m_Stat;
		int m_Stages;
	};

	std::vector<StatStage> m_StatStages;
	uint8_t m_Chance;
	eChangeAffects m_AffectedParty;
};

class Move
{
public:
	Move() = default;

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

	struct Outcome
	{
		bool m_MoveMissed;
		bool m_IsCriticalHit;
		uint16_t m_Damage;
		float m_TypeMultiplier;

		struct StatChangeOutcome
		{
			struct SucceededStage
			{
				StatChange::StatStage m_Stage;
				bool m_Succeeded;
			};

			std::vector<SucceededStage> m_AttackerStatChanges;
			std::vector<SucceededStage> m_DefenderStatChanges;
		};

		std::optional<StatChangeOutcome> m_StatChangeOutcome;
	};

	using stat_change_outcome = Outcome::StatChangeOutcome::SucceededStage;

	Outcome Use(Entity& attacker, Entity& defender);

	unsigned GetPPRemaining() const;
	unsigned GetMaxPP() const;

	monster_type GetType() const;

	bool IsValid() const;

	hash_type GetNameStringTableID() const;

private:
	bool m_valid = false;
	uint32_t m_id;
	hash_type m_stringTableID;
	hash_type m_descriptionStringTableID;
	monster_type m_type;
	eMoveCategory m_category;
	unsigned m_powerPoints;
	unsigned m_maxPowerPoints;
	int m_priority;
	eMoveTarget m_target;
	eEffectCategory m_effectCategory;

	std::optional<unsigned> m_power;
	std::optional<uint8_t> m_accuracy; // 0-100
	std::optional<MoveStatus> m_status;
	std::optional<StatChange> m_statChange;
};


#endif // MOVE_H
