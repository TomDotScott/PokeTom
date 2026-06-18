#ifndef BATTLELOOPLAYER_H
#define BATTLELOOPLAYER_H

#include <cstdint>
#include <queue>

#include "UILayer.h"
#include "../Move.h"

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
	bool m_playerSwitchedOut;
	bool m_opponentSwitchedOut;

	struct BattleMessage
	{
		std::string m_MessageName;
		std::function<void()> m_OnShow;
	};
	std::queue<BattleMessage> m_messageQueue;

	void DoTurn(PocketMonsterEntity* attacker, uint8_t selectedMoveIdx, PocketMonsterEntity* defender);

	void ShowNextMessage();
	void ShowMoveNameText(const PocketMonsterEntity* monster, uint8_t moveIdx) const;
	void ShowFaintText(const PocketMonsterEntity* monster) const;

	// TODO: Moves can miss for more than just accuracy (defending monster's evasion stat, attacks failing when used in sequence - i.e. trying to burn a monster that's already burned - and protect/detect. We need to have a case for each of these situations!
	void ShowMissText(const PocketMonsterEntity* monster) const;
	void ShowCriticalHitText() const;
	void ShowEffectivenessText(float moveOutcome) const;
	Move::Outcome UseMove(PocketMonsterEntity* attacker, PocketMonsterEntity* defender, uint8_t moveIdx) const;
};
#endif // BATTLELOOPLAYER_H
