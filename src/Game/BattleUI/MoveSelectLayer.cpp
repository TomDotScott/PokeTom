#include "MoveSelectLayer.h"

#include "../BattleState.h"
#include "../../Engine/Asserts.h"
#include "../../Engine/UI/UiManager.h"
#include "../../Engine/UI/UiPanel.h"
#include "../../Engine/UI/UiText.h"
#include "../Monsters/PocketMonsterEntity.h"

MoveSelectLayer::MoveSelectLayer() :
	m_playerMonster(nullptr),
	m_opponentMonster(nullptr),
	m_validMoves(),
	m_selectedMove(eSelection::Move1)
{
}

UILayer::LayerResult MoveSelectLayer::GetLayerResult() const
{
	return {
		.m_NextLayer = BattleLoop,
		.m_ChosenMoveIndex = static_cast<uint8_t>(m_selectedMove),
	};
}

void MoveSelectLayer::OnNavigateButtonPressed(const eUILayerNavigateButtons button)
{
	const bool upDown = button & (UP | DOWN);
	const bool leftRight = button & (LEFT | RIGHT);

	if (upDown)
	{
		switch (m_selectedMove)
		{
		case eSelection::Move1:
			if (m_validMoves[static_cast<size_t>(eSelection::Move3)])
			{
				OnSelectedMoveChanged(eSelection::Move3);
			}
			break;
		case eSelection::Move2:
			if (m_validMoves[static_cast<size_t>(eSelection::Move4)])
			{
				OnSelectedMoveChanged(eSelection::Move4);
			}
			break;
		case eSelection::Move3:
			if (m_validMoves[static_cast<size_t>(eSelection::Move1)])
			{
				OnSelectedMoveChanged(eSelection::Move1);
			}
			break;
		case eSelection::Move4:
			if (m_validMoves[static_cast<size_t>(eSelection::Move2)])
			{
				OnSelectedMoveChanged(eSelection::Move2);
			}
			break;
		}
	}
	else if (leftRight)
	{
		switch (m_selectedMove)
		{
		case eSelection::Move1:
			if (m_validMoves[static_cast<size_t>(eSelection::Move2)])
			{
				OnSelectedMoveChanged(eSelection::Move2);
			}
			break;
		case eSelection::Move2:
			if (m_validMoves[static_cast<size_t>(eSelection::Move1)])
			{
				OnSelectedMoveChanged(eSelection::Move1);
			}
			break;
		case eSelection::Move3:
			if (m_validMoves[static_cast<size_t>(eSelection::Move4)])
			{
				OnSelectedMoveChanged(eSelection::Move4);
			}
			break;
		case eSelection::Move4:
			if (m_validMoves[static_cast<size_t>(eSelection::Move3)])
			{
				OnSelectedMoveChanged(eSelection::Move3);
			}
			break;
		}
	}
}

void MoveSelectLayer::OnSelectButtonPressed()
{
	ASSERT(this->m_playerMonster != nullptr);

	const MoveComponent* playerMoveComponent = this->m_playerMonster->GetComponent<MoveComponent>();
	ASSERT(playerMoveComponent);

	m_finished = playerMoveComponent->CanUseMove(static_cast<uint8_t>(m_selectedMove));
}

void MoveSelectLayer::OnActivate(const BattleState& state, const LayerResult& prevLayerResult)
{
	UILayer::OnActivate(state, prevLayerResult);

	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	battleUI->GetChild("TEXT_BOX")->OnDeactivate();

	auto* optionsUI = dynamic_cast<UiPanel*>(battleUI->GetChild(OPTIONS_PANEL_NAME));
	ASSERT(optionsUI != nullptr);

	auto* moveUI = dynamic_cast<UiPanel*>(battleUI->GetChild(MOVES_PANEL_NAME));
	ASSERT(moveUI != nullptr);

	optionsUI->OnDeactivate();
	moveUI->OnActivate();


	this->m_playerMonster = state.GetGameContext().m_Entities.Get<PocketMonsterEntity>(state.GetPlayerMonsterEntityID());
	ASSERT(this->m_playerMonster != nullptr);

	this->m_opponentMonster = state.GetGameContext().m_Entities.Get<PocketMonsterEntity>(state.GetOpponentMonsterEntityID());
	ASSERT(this->m_opponentMonster != nullptr);

	// Set up the move text
	MoveComponent* moveComponent = this->m_playerMonster->GetComponent<MoveComponent>();
	ASSERT(moveComponent != nullptr);

	static constexpr std::array uiComponentNames{
		"MOVE_1_TEXT",
		"MOVE_2_TEXT",
		"MOVE_3_TEXT",
		"MOVE_4_TEXT",
	};

	for (size_t i = 0; i < MOVE_COUNT; ++i)
	{
		const Move& move = moveComponent->GetMove(i);

		bool isValid = move.IsValid();
		m_validMoves[i] = isValid;

		UiText* moveText = dynamic_cast<UiText*>(moveUI->GetChild(uiComponentNames[i]));
		ASSERT(moveText != nullptr);

		if (isValid)
		{
			moveText->SetText(move.GetNameStringTableID());
		}
		else
		{
			moveText->SetText(" ");
		}
	}

	OnSelectedMoveChanged(eSelection::Move1);
}

void MoveSelectLayer::OnDeactivate()
{
	UILayer::OnDeactivate();

	const auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	battleUI->GetChild("TEXT_BOX")->OnActivate();

	auto* moveUI = dynamic_cast<UiPanel*>(battleUI->GetChild(MOVES_PANEL_NAME));
	ASSERT(moveUI != nullptr);

	moveUI->OnDeactivate();
}

void MoveSelectLayer::OnSelectedMoveChanged(const eSelection newMove)
{
	m_selectedMove = newMove;

	const auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	const auto* moveSelectUI = dynamic_cast<UiPanel*>(battleUI->GetChild(MOVES_PANEL_NAME));
	ASSERT(moveSelectUI != nullptr);

	auto* move1 = moveSelectUI->GetChild("MOVE_1_ARROW");
	ASSERT(move1 != nullptr);

	auto* move2 = moveSelectUI->GetChild("MOVE_2_ARROW");
	ASSERT(move2 != nullptr);

	auto* move3 = moveSelectUI->GetChild("MOVE_3_ARROW");
	ASSERT(move3 != nullptr);

	auto* move4 = moveSelectUI->GetChild("MOVE_4_ARROW");
	ASSERT(move4 != nullptr);

	move1->OnDeactivate();
	move2->OnDeactivate();
	move3->OnDeactivate();
	move4->OnDeactivate();

	switch (newMove)
	{
	case eSelection::Move1:
		move1->OnActivate();
		break;
	case eSelection::Move2:
		move2->OnActivate();
		break;
	case eSelection::Move3:
		move3->OnActivate();
		break;
	case eSelection::Move4:
		move4->OnActivate();
		break;
	}
}
