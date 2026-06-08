#ifndef BATTLELOOPLAYER_H
#define BATTLELOOPLAYER_H

#include <cstdint>

#include "UILayer.h"

class PocketMonsterEntity;

class BattleLoopLayer final : public UILayer
{
public:
	BattleLoopLayer();
	LayerResult GetLayerResult() const override;
	void OnSelectButtonPressed() override;
	void OnActivate(const BattleState& state, const LayerResult& prevLayerResult) override;
	void OnDeactivate() override;
	void OnNavigateButtonPressed(eUILayerNavigateButtons button) override;

private:
	PocketMonsterEntity* m_playerMonster;
	PocketMonsterEntity* m_opponentMonster;

	uint8_t m_playerChosenMoveIdx;

	enum class eBattleFlow
	{
		PlayerSwitch,
		OpponentSwitch,
		PlayerMoveName,
		PlayerMoveEffectiveness,
		OpponentFaintOrSwitch,
		OpponentMoveName,
		OpponentMoveEffectiveness,
		PlayerFaint
	};

	eBattleFlow m_currentState;

	void TransitionToFlowState(eBattleFlow state);
};
#endif // BATTLELOOPLAYER_H
