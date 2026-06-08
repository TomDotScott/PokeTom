#include "BattleLoopLayer.h"

#include <iostream>
#include <magic_enum.hpp>
#include <frozen/unordered_map.h>

#include "../BattleState.h"
#include "../../Engine/UI/UiManager.h"
#include "../Monsters/PocketMonsterEntity.h"
#include "../DialogueBox.h"
#include "../../Engine/Stringtable.h"

constexpr static frozen::unordered_map<eTypeEffectiveness, const char*, 4> EFFECTIVENESS_STRING_IDs = {
	{ eTypeEffectiveness::Effective, "" },
	{ eTypeEffectiveness::Immune, "MOVE_IMMUNE" },
	{ eTypeEffectiveness::NotVeryEffective, "MOVE_NOT_VERY_EFFECTIVE" },
	{ eTypeEffectiveness::SuperEffective, "MOVE_SUPER_EFFECTIVE" }
};

BattleLoopLayer::BattleLoopLayer() :
	m_playerMonster(nullptr),
	m_opponentMonster(nullptr),
	m_playerChosenMoveIdx(0xFF),
	m_opponentChosenMoveIdx(0xFF),
	m_playerSwitchedOut(false),
	m_opponentSwitchedOut(false),
	m_currentState(eBattleFlow::PlayerSwitch)
{
}

UILayer::LayerResult BattleLoopLayer::GetLayerResult() const
{
	return { .m_NextLayer = OptionSelect };
}

void BattleLoopLayer::OnActivate(const BattleState& state, const LayerResult& prevLayerResult)
{
	UILayer::OnActivate(state, prevLayerResult);

	ASSERT(prevLayerResult.m_ChosenMoveIndex.has_value());
	m_playerChosenMoveIdx = prevLayerResult.m_ChosenMoveIndex.value();

	m_opponentChosenMoveIdx = 0;

	m_playerMonster = state.GetGameContext().m_Entities.Get<PocketMonsterEntity>(
		state.GetPlayerMonsterEntityID());
	ASSERT(m_playerMonster != nullptr);

	m_opponentMonster = state.GetGameContext().m_Entities.Get<PocketMonsterEntity>(
		state.GetOpponentMonsterEntityID());
	ASSERT(m_opponentMonster != nullptr);

	m_playerSwitchedOut = false;
	m_opponentSwitchedOut = false;

	if (m_playerSwitchedOut)
	{
		TransitionToFlowState(eBattleFlow::PlayerSwitch);
	}
	else if (m_opponentSwitchedOut)
	{
		TransitionToFlowState(eBattleFlow::OpponentSwitch);
	}
	else
	{
		TransitionToFlowState(eBattleFlow::PlayerMoveName);
	}
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
	TransitionToFlowState(GetNextFlowState());
}

BattleLoopLayer::eBattleFlow BattleLoopLayer::GetNextFlowState() const
{
	switch (m_currentState)
	{
	case eBattleFlow::PlayerSwitch:
		return eBattleFlow::OpponentSwitch;

	case eBattleFlow::OpponentSwitch:
		return eBattleFlow::PlayerMoveName;

	case eBattleFlow::PlayerMoveName:
		return eBattleFlow::PlayerMoveEffectiveness;

	case eBattleFlow::PlayerMoveEffectiveness:
		return m_opponentMonster->IsFainted()
			       ? eBattleFlow::OpponentFaint
			       : eBattleFlow::OpponentMoveName;

	case eBattleFlow::OpponentFaint:
		return eBattleFlow::OpponentMoveName;

	case eBattleFlow::OpponentMoveName:
		return eBattleFlow::OpponentMoveEffectiveness;

	case eBattleFlow::OpponentMoveEffectiveness:
		{
			if (m_playerMonster->IsFainted())
			{
				return eBattleFlow::PlayerFaint;
			}

			return eBattleFlow::PlayerMoveName;
		}

	case eBattleFlow::PlayerFaint:
		return eBattleFlow::PlayerMoveName;
	}

	return eBattleFlow::PlayerMoveName;
}

void BattleLoopLayer::TransitionToFlowState(const eBattleFlow state)
{
#if BUILD_DEBUG
	std::cout << "Transitioning to state " << magic_enum::enum_name(state) << "\n";
#endif

	DialogueBox::SetVisible(true);

	switch (state)
	{
	case eBattleFlow::PlayerSwitch:
		if (!m_playerSwitchedOut)
		{
			TransitionToFlowState(eBattleFlow::OpponentSwitch);
			return;
		}
	// TODO: ShowSwitchText(m_playerMonster);
		break;

	case eBattleFlow::OpponentSwitch:
		if (!m_opponentSwitchedOut)
		{
			TransitionToFlowState(eBattleFlow::PlayerMoveName);
			return;
		}
	// TODO: ShowSwitchText(m_opponentMonster);
		break;

	case eBattleFlow::PlayerMoveName:
		ShowMoveNameText(m_playerMonster, m_playerChosenMoveIdx);
		break;

	case eBattleFlow::PlayerMoveEffectiveness:
		{
			const eTypeEffectiveness effectiveness = UseMove(m_playerMonster, m_opponentMonster);
			if (effectiveness == eTypeEffectiveness::Effective)
			{
				TransitionToFlowState(eBattleFlow::OpponentMoveName);
				return;
			}
			ShowEffectivenessText(effectiveness);
		}
		break;

	case eBattleFlow::OpponentFaint:
		ShowFaintText(m_opponentMonster);
		break;

	case eBattleFlow::OpponentMoveName:
		ShowMoveNameText(m_opponentMonster, m_opponentChosenMoveIdx);
		break;

	case eBattleFlow::OpponentMoveEffectiveness:
		{
			const eTypeEffectiveness effectiveness = UseMove(m_opponentMonster, m_playerMonster);
			if (effectiveness == eTypeEffectiveness::Effective)
			{
				// round over!
				if (!m_playerMonster->IsFainted())
				{
					m_finished = true;
				}

				return;
			}

			ShowEffectivenessText(effectiveness);
		}
		break;

	case eBattleFlow::PlayerFaint:
		ShowFaintText(m_playerMonster);
		m_finished = true;
		break;
	}

	m_currentState = state;
}

void BattleLoopLayer::ShowMoveNameText(const PocketMonsterEntity* monster, const uint8_t moveIdx) const
{
	const std::string monsterName = STRINGTABLE->GetString(HASH("MONSTER_NAME"), monster->GetNameStringID());
	const std::string moveName = monster->GetMoveName(moveIdx);
	DialogueBox::SetText(STRINGTABLE->GetDynamicString(HASH("MONSTER_MOVE_NAME"), monsterName, moveName).c_str());
}

void BattleLoopLayer::ShowFaintText(const PocketMonsterEntity* monster) const
{
	const std::string monsterName = STRINGTABLE->GetString(HASH("MONSTER_NAME"), monster->GetNameStringID());
	DialogueBox::SetText(STRINGTABLE->GetDynamicString(HASH("MONSTER_FAINT"), monsterName).c_str());
}

void BattleLoopLayer::ShowEffectivenessText(const eTypeEffectiveness effectiveness) const
{
	DialogueBox::SetText(STRINGTABLE->GetString(
		HASH("BATTLE"), EFFECTIVENESS_STRING_IDs.at(effectiveness)).c_str());
}

eTypeEffectiveness BattleLoopLayer::UseMove(PocketMonsterEntity* attacker, PocketMonsterEntity* defender) const
{
	ASSERT(m_playerChosenMoveIdx < MOVE_COUNT);
	ASSERT(attacker != nullptr);
	ASSERT(defender != nullptr);

	auto* moveComponent = attacker->GetComponent<MoveComponent>();
	ASSERT(moveComponent && moveComponent->CanUseMove(m_playerChosenMoveIdx));

	return moveComponent->UseMove(m_playerChosenMoveIdx, *defender);
}
