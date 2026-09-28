#ifndef RUNAWAYLAYER_H
#define RUNAWAYLAYER_H

#include "../BattleContext.h"
#include "UILayer.h"

class BattleState;

class RunAwayLayer final : public UILayer
{
public:
	RunAwayLayer() = default;
	LayerResult GetLayerResult() const override;
	void OnNavigateButtonPressed(eUILayerNavigateButtons button) override;
	void OnSelectButtonPressed() override;
	void OnBackButtonPressed() override;
	void OnMoreInfoButtonPressed() override;
	void OnActivate(const BattleState& state, const LayerResult& prevLayerResult) override;
	void OnDeactivate() override;

private:
	BattleEndContext m_endContext;
};
#endif // RUNAWAYLAYER_H
