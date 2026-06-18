#include "BattleLoopLayer.h"

#include <iostream>
#include <frozen/unordered_map.h>

#include "../BattleState.h"
#include "../DialogueBox.h"
#include "../../Engine/Stringtable.h"
#include "../Monsters/PocketMonsterEntity.h"

namespace
{
	const hash_type MONSTER_NAME_GROUP = HASH("MONSTER_NAME");
}

BattleLoopLayer::BattleLoopLayer() :
	m_playerSwitchedOut(false),
	m_opponentSwitchedOut(false)
{
}

UILayer::LayerResult BattleLoopLayer::GetLayerResult() const
{
	return { .m_NextLayer = OptionSelect };
}

void BattleLoopLayer::OnActivate(const BattleState& state, const LayerResult& prevLayerResult)
{
	UILayer::OnActivate(state, prevLayerResult);

	DialogueBox::SetVisible(true);

	const EntityRegistry& entities = state.GetGameContext().m_Entities;

	PocketMonsterEntity* playerMonster = entities.Get<PocketMonsterEntity>(state.GetPlayerMonsterEntityID());
	ASSERT(playerMonster != nullptr);

	PocketMonsterEntity* opponentMonster = entities.Get<PocketMonsterEntity>(state.GetOpponentMonsterEntityID());
	ASSERT(opponentMonster != nullptr);

	// TODO:
	m_playerSwitchedOut = false;
	m_opponentSwitchedOut = false;

	ASSERT(prevLayerResult.m_ChosenMoveIndex.has_value());

	// Determine who is first or second
	uint8_t firstMonsterMoveIdx;
	uint8_t secondMonsterMoveIdx;

	// TODO: Properly choose opponent move rather than always 0!
	PocketMonsterEntity* first;
	PocketMonsterEntity* second;
	if (playerMonster->GetStats().m_Speed > opponentMonster->GetStats().m_Speed)
	{
		first = playerMonster;
		firstMonsterMoveIdx = prevLayerResult.m_ChosenMoveIndex.value();

		second = opponentMonster;
		secondMonsterMoveIdx = 0;
	}
	else
	{
		first = opponentMonster;
		firstMonsterMoveIdx = 0;

		second = playerMonster;
		secondMonsterMoveIdx = prevLayerResult.m_ChosenMoveIndex.value();
	}

	ASSERT(first != nullptr);
	ASSERT(second != nullptr);

	DoTurn(first, firstMonsterMoveIdx, second);
	DoTurn(second, secondMonsterMoveIdx, first);

	// Add any messages for weather effects
	// Add any messages for burn/poison chip damage
	// Faint check for end of round effect
	if (first->IsFainted())
	{
		m_messageQueue.emplace("FASTEST FAINTED", [this, first]()
		{
			ShowFaintText(first);
		});
	}

	if (second->IsFainted())
	{
		m_messageQueue.emplace("SLOWER FAINTED", [this, second]()
		{
			ShowFaintText(second);
		});
	}


	ShowNextMessage();
}

void BattleLoopLayer::OnDeactivate()
{
	UILayer::OnDeactivate();
	DialogueBox::SetVisible(false);
}

void BattleLoopLayer::OnNavigateButtonPressed(eUILayerNavigateButtons button)
{
}

void BattleLoopLayer::OnSelectButtonPressed()
{
	ShowNextMessage();
}

void BattleLoopLayer::DoTurn(PocketMonsterEntity* attacker,
                             const uint8_t selectedMoveIdx,
                             PocketMonsterEntity* defender
)
{
	// Perform the move and then queue the UI messages...
	Move::Outcome outcome = UseMove(attacker, defender, selectedMoveIdx);

	// X used Y...
	m_messageQueue.emplace(("MOVE NAME"), [this, attacker, selectedMoveIdx]()
	{
		ShowMoveNameText(attacker, selectedMoveIdx);
	});

	// The move hits or misses...
	if (outcome.m_MoveMissed)
	{
		m_messageQueue.emplace(("THE MOVE MISSED!"), [this, attacker]()
		{
			ShowMissText(attacker);
		});

		return;
	}

	// Effectiveness message (and healthbar animation)...
	const float effectiveness = outcome.m_TypeMultiplier;
	if (effectiveness != 1.0f)
	{
		m_messageQueue.emplace(("IMMUNE/VERY/NOT VERY EFFECTIVE TEXT"), [this, effectiveness]()
		{
			ShowEffectivenessText(effectiveness);
		});
	}

	// Was it a crit?
	if (outcome.m_IsCriticalHit)
	{
		m_messageQueue.emplace(("CRITICAL HIT!"), [this]()
		{
			ShowCriticalHitText();
		});
	}

	// Any Stat Changes
	// Any Status Conditions
	// Recoil or Health Drain/Gain?
	// Faint Check
	if (attacker->IsFainted())
	{
		m_messageQueue.emplace("ATTACKER FAINTED", [this, attacker]()
		{
			ShowFaintText(attacker);
		});
	}

	if (defender->IsFainted())
	{
		m_messageQueue.emplace("DEFENDER FAINTED", [this, defender]()
		{
			ShowFaintText(defender);
		});
	}
}

void BattleLoopLayer::ShowNextMessage()
{
	if (m_messageQueue.empty())
	{
		m_finished = true;
		return;
	}

	const auto& nextMessage = m_messageQueue.front();
	nextMessage.m_OnShow();

	m_messageQueue.pop();
}

void BattleLoopLayer::ShowMoveNameText(const PocketMonsterEntity* monster, const uint8_t moveIdx) const
{
	const std::string monsterName = STRINGTABLE->GetString(MONSTER_NAME_GROUP, monster->GetNameStringID());
	const std::string moveName = monster->GetMoveName(moveIdx);
	DialogueBox::SetText(STRINGTABLE->GetDynamicString(HASH("MONSTER_MOVE_NAME"), monsterName, moveName).c_str());
}

void BattleLoopLayer::ShowFaintText(const PocketMonsterEntity* monster) const
{
	const std::string monsterName = STRINGTABLE->GetString(MONSTER_NAME_GROUP, monster->GetNameStringID());
	DialogueBox::SetText(STRINGTABLE->GetDynamicString(HASH("MONSTER_FAINT"), monsterName).c_str());
}

void BattleLoopLayer::ShowMissText(const PocketMonsterEntity* monster) const
{
	const std::string monsterName = STRINGTABLE->GetString(MONSTER_NAME_GROUP, monster->GetNameStringID());
	DialogueBox::SetText(STRINGTABLE->GetDynamicString(HASH("MONSTER_ATTACK_MISS"), monsterName).c_str());
}

void BattleLoopLayer::ShowCriticalHitText() const
{
	DialogueBox::SetText(STRINGTABLE->GetString(HASH("BATTLE"), HASH("MOVE_CRITICAL")).c_str());
}

void BattleLoopLayer::ShowEffectivenessText(const float moveOutcome) const
{
	hash_type stringID = DEFAULT_HASH;
	if (moveOutcome == 0.f)
	{
		stringID = HASH("MOVE_IMMUNE");
	}
	else if (moveOutcome <= 0.5f)
	{
		stringID = HASH("MOVE_NOT_VERY_EFFECTIVE");
	}
	else if (moveOutcome > 1.0f)
	{
		stringID = HASH("MOVE_SUPER_EFFECTIVE");
	}

	ASSERT(stringID != DEFAULT_HASH);

	DialogueBox::SetText(STRINGTABLE->GetString(HASH("BATTLE"), stringID).c_str());
}

Move::Outcome BattleLoopLayer::UseMove(PocketMonsterEntity* attacker,
                                       PocketMonsterEntity* defender,
                                       uint8_t moveIdx) const
{
	ASSERT(moveIdx < MOVE_COUNT);
	ASSERT(attacker != nullptr);
	ASSERT(defender != nullptr);

	auto* moveComponent = attacker->GetComponent<MoveComponent>();
	ASSERT(moveComponent && moveComponent->CanUseMove(moveIdx));


	const Move::Outcome outcome = moveComponent->UseMove(moveIdx, *defender);

#if BUILD_DEBUG
	std::cout << STRINGTABLE->GetString(MONSTER_NAME_GROUP, attacker->GetNameStringID()) << " stats:\n";
	attacker->GetStats().Log();

	std::cout << "\n";

	std::cout << STRINGTABLE->GetString(MONSTER_NAME_GROUP, defender->GetNameStringID()) << " stats:\n";
	defender->GetStats().Log();
#endif

	return outcome;
}
