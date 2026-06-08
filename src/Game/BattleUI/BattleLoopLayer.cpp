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
	m_playerSwitchedOut(false),
	m_opponentSwitchedOut(false),
	m_currentState(eBattleFlow::PlayerSwitch)
{
}

UILayer::LayerResult BattleLoopLayer::GetLayerResult() const
{
	return { .m_NextLayer = OptionSelect };
}

void BattleLoopLayer::OnSelectButtonPressed()
{
	// TODO: Change order of attack based on speed of the monsters!
	switch (m_currentState)
	{
	case eBattleFlow::PlayerSwitch:
		TransitionToFlowState(eBattleFlow::PlayerMoveName);
		break;
	case eBattleFlow::OpponentSwitch:
		TransitionToFlowState(eBattleFlow::PlayerMoveName);
		break;
	case eBattleFlow::PlayerMoveName:
		TransitionToFlowState(eBattleFlow::PlayerMoveEffectiveness);
		break;
	case eBattleFlow::PlayerMoveEffectiveness:
		TransitionToFlowState(eBattleFlow::PlayerFaint);
		break;
	case eBattleFlow::OpponentFaint:
		TransitionToFlowState(eBattleFlow::OpponentMoveName);
		break;
	case eBattleFlow::OpponentMoveName:
		TransitionToFlowState(eBattleFlow::OpponentMoveEffectiveness);
		break;
	case eBattleFlow::OpponentMoveEffectiveness:
		TransitionToFlowState(eBattleFlow::PlayerFaint);
		break;
	case eBattleFlow::PlayerFaint:
		m_finished = true;
		break;
	}
}

void BattleLoopLayer::OnActivate(const BattleState& state, const LayerResult& prevLayerResult)
{
	UILayer::OnActivate(state, prevLayerResult);

	ASSERT(prevLayerResult.m_ChosenMoveIndex.has_value());
	this->m_playerChosenMoveIdx = prevLayerResult.m_ChosenMoveIndex.value();

	this->m_playerMonster = state.GetGameContext().m_Entities.Get<
		PocketMonsterEntity>(state.GetPlayerMonsterEntityID());
	ASSERT(this->m_playerMonster != nullptr);

	this->m_opponentMonster = state.GetGameContext().m_Entities.Get<PocketMonsterEntity>(
		state.GetOpponentMonsterEntityID());
	ASSERT(this->m_opponentMonster != nullptr);

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

void BattleLoopLayer::TransitionToFlowState(const eBattleFlow state)
{
#if BUILD_DEBUG
	std::cout << "Transitioning to state " << magic_enum::enum_name(state) << "\n";
#endif

	DialogueBox::SetVisible(true);

	switch (state)
	{
	case eBattleFlow::PlayerSwitch:
		{
			if (!m_playerSwitchedOut)
			{
				TransitionToFlowState(eBattleFlow::OpponentSwitch);
				return;
			}
		}
		break;
	case eBattleFlow::OpponentSwitch:
		{
			if (!m_opponentSwitchedOut)
			{
				TransitionToFlowState(eBattleFlow::PlayerMoveName);
				return;
			}
		}
		break;
	case eBattleFlow::PlayerMoveName:
		{
			const std::string monsterName = STRINGTABLE->GetString(
				HASH("MONSTER_NAME"),
				m_playerMonster->GetNameStringID()
			);

			const std::string moveName = m_playerMonster->GetMoveName(m_playerChosenMoveIdx);

			const std::string text = STRINGTABLE->GetDynamicString(
				HASH("MONSTER_MOVE_NAME"),
				monsterName,
				moveName
			);

			DialogueBox::SetText(text.c_str());
		}
		break;
	case eBattleFlow::PlayerMoveEffectiveness:
		{
			const eTypeEffectiveness effectiveness = UseMove(m_playerMonster, m_opponentMonster);

			if (effectiveness != eTypeEffectiveness::Effective)
			{
				DialogueBox::SetText(STRINGTABLE->GetString(
						HASH("BATTLE"),
						EFFECTIVENESS_STRING_IDs.at(effectiveness)).c_str()
				);

				if (m_opponentMonster->IsFainted())
				{
					TransitionToFlowState(eBattleFlow::OpponentFaint);
					return;
				}
			}
			else
			{
				TransitionToFlowState(eBattleFlow::OpponentMoveName);
			}
		}
		break;
	case eBattleFlow::OpponentFaint:
		{
			const std::string monsterName = STRINGTABLE->GetString(
				HASH("MONSTER_NAME"),
				m_opponentMonster->GetNameStringID()
			);

			const std::string text = STRINGTABLE->GetDynamicString(
				HASH("MONSTER_FAINT"),
				monsterName
			);

			DialogueBox::SetText(text.c_str());
		}
		break;
	case eBattleFlow::OpponentMoveName:
		{
			const std::string monsterName = STRINGTABLE->GetString(
				HASH("MONSTER_NAME"),
				m_opponentMonster->GetNameStringID()
			);

			const std::string moveName = m_opponentMonster->GetMoveName(m_playerChosenMoveIdx);

			const std::string text = STRINGTABLE->GetDynamicString(
				HASH("MONSTER_MOVE_NAME"),
				monsterName,
				moveName
			);

			DialogueBox::SetText(text.c_str());
		}
		break;
	case eBattleFlow::OpponentMoveEffectiveness:
		{
			const eTypeEffectiveness effectiveness = UseMove(m_opponentMonster, m_playerMonster);

			if (effectiveness != eTypeEffectiveness::Effective)
			{
				DialogueBox::SetText(STRINGTABLE->GetString(
						HASH("BATTLE"),
						EFFECTIVENESS_STRING_IDs.at(effectiveness)).c_str()
				);

				if (m_playerMonster->IsFainted())
				{
					TransitionToFlowState(eBattleFlow::OpponentFaint);
					return;
				}
			}
			else
			{
				TransitionToFlowState(eBattleFlow::OpponentMoveName);
			}
		}
		break;
	case eBattleFlow::PlayerFaint:
		{
			const std::string monsterName = STRINGTABLE->GetString(
				HASH("MONSTER_NAME"),
				m_playerMonster->GetNameStringID()
			);

			const std::string text = STRINGTABLE->GetDynamicString(
				HASH("MONSTER_FAINT"),
				monsterName
			);

			DialogueBox::SetText(text.c_str());
		}
		break;
	}

	m_currentState = state;
}

eTypeEffectiveness BattleLoopLayer::UseMove(PocketMonsterEntity* attacker, PocketMonsterEntity* defender) const
{
	// TODO: Opponent chooses random move! Oh well, can copy the player for now!
	ASSERT(m_playerChosenMoveIdx < MOVE_COUNT);
	ASSERT(attacker != nullptr);
	ASSERT(defender != nullptr);

	auto* moveComponent = attacker->GetComponent<MoveComponent>();

	ASSERT(moveComponent && moveComponent->CanUseMove(m_playerChosenMoveIdx));

	return moveComponent->UseMove(m_playerChosenMoveIdx, *defender);
}
