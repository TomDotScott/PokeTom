#ifndef MOVESELECTLAYER_H
#define MOVESELECTLAYER_H

#include "../MoveComponent.h"

#include "UILayer.h"
#include "../../Engine/StringUtils.h"

class PocketMonsterEntity;

class MoveSelectLayer final : public UILayer
{
public:
	MoveSelectLayer();
	LayerResult GetLayerResult() const override;
	void OnNavigateButtonPressed(eUILayerNavigateButtons button) override;
	void OnSelectButtonPressed() override;
	void OnBackButtonPressed() override;
	void OnMoreInfoButtonPressed() override;
	void OnActivate(const BattleState& state, const LayerResult& prevLayerResult) override;
	void OnDeactivate() override;

private:
	PocketMonsterEntity* m_playerMonster;
	PocketMonsterEntity* m_opponentMonster;

	std::array<bool, MOVE_COUNT> m_validMoves;

	enum class eSelection : uint8_t
	{
		Move1,
		Move2,
		Move3,
		Move4
	};
	eSelection m_selectedMove;

	bool m_backRequested;
	bool m_isShowingDescriptionText;
	unsigned m_descriptionIndex;
	string_utils::text_pages m_currentMoveDescription;

	void OnSelectedMoveChanged(eSelection newMove);
	void ShowMoveDescription();
	void UpdateDescriptionText();
	void DismissMoveDescription();
};
#endif // MOVESELECTLAYER_H
