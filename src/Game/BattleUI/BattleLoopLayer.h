#ifndef BATTLELOOPLAYER_H
#define BATTLELOOPLAYER_H

#include <cstdint>

#include "UILayer.h"
#include "../Monsters/MonsterTypes.h"

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
	uint8_t m_opponentChosenMoveIdx;

	bool m_playerSwitchedOut;
	bool m_opponentSwitchedOut;

	enum class eBattleFlow
	{
		PlayerSwitch,
		OpponentSwitch,
		PlayerMoveName,
		PlayerMoveEffectiveness,
		OpponentFaint,
		OpponentMoveName,
		OpponentMoveEffectiveness,
		PlayerFaint
	};

	eBattleFlow m_currentState;

	void TransitionToFlowState(eBattleFlow state);
	void ShowMoveNameText(const PocketMonsterEntity* monster, uint8_t moveIdx) const;
	void ShowFaintText(const PocketMonsterEntity* monster) const;
	void ShowEffectivenessText(eTypeEffectiveness effectiveness) const;
	eTypeEffectiveness UseMove(PocketMonsterEntity* attacker, PocketMonsterEntity* defender) const;
	eBattleFlow GetNextFlowState() const;
};
#endif // BATTLELOOPLAYER_H
