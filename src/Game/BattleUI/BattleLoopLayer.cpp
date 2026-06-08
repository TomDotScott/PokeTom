#include "BattleLoopLayer.h"

#include <iostream>
#include <magic_enum.hpp>

#include "../BattleState.h"
#include "../../Engine/UI/UiManager.h"
#include "../Monsters/PocketMonsterEntity.h"
#include "../DialogueBox.h"
#include "../../Engine/Stringtable.h"


BattleLoopLayer::BattleLoopLayer() :
	m_playerMonster(nullptr),
	m_opponentMonster(nullptr),
	m_playerChosenMoveIdx(0xFF),
	m_currentState(eBattleFlow::PlayerSwitch)
{
}

UILayer::LayerResult BattleLoopLayer::GetLayerResult() const
{
	return { .m_NextLayer = OptionSelect };
}

void BattleLoopLayer::OnSelectButtonPressed()
{
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
		TransitionToFlowState(eBattleFlow::OpponentFaintOrSwitch);
		break;
	case eBattleFlow::OpponentFaintOrSwitch:
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

	bool hasPlayerSwitched = false;
	bool hasOpponentSwitched = false;

	if (hasPlayerSwitched)
	{
		TransitionToFlowState(eBattleFlow::PlayerSwitch);
	}
	else if (hasOpponentSwitched)
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
		break;
	case eBattleFlow::OpponentSwitch:
		break;
	case eBattleFlow::PlayerMoveName:
		{
			const std::string monsterName = STRINGTABLE->GetString(
				"MONSTER_NAME",
				m_playerMonster->GetNameStringID()
			);

			const std::string moveName = m_playerMonster->GetMoveName(m_playerChosenMoveIdx);

			const std::string text = STRINGTABLE->GetDynamicString(
				HASH("PLAYER_MOVE_NAME"),
				monsterName,
				moveName
			);

			DialogueBox::SetText(text.c_str());
		}
		break;
	case eBattleFlow::PlayerMoveEffectiveness:
		break;
	case eBattleFlow::OpponentFaintOrSwitch:
		break;
	case eBattleFlow::OpponentMoveName:
		break;
	case eBattleFlow::OpponentMoveEffectiveness:
		break;
	case eBattleFlow::PlayerFaint:
		break;
	}

	m_currentState = state;
}
