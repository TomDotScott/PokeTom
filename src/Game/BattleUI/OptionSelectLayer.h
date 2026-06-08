#ifndef OPTIONSELECTLAYER_H
#define OPTIONSELECTLAYER_H

#include <cstdint>

#include "UILayer.h"

class BattleState;

class OptionSelectLayer final : public UILayer
{
public:
	OptionSelectLayer();
	LayerResult GetLayerResult() const override;
	void OnNavigateButtonPressed(eUILayerNavigateButtons button) override;
	void OnSelectButtonPressed() override;
	void OnActivate(const BattleState& state, const LayerResult& prevLayerResult) override;
	void OnDeactivate() override;

private:
	enum class eSelectedOption : uint8_t
	{
		Fight,
		Monsters,
		Bag,
		Run
	};

	eSelectedOption m_selectedOption;

	void OnSelectedOptionChanged(eSelectedOption newOption);
};
#endif // OPTIONSELECTLAYER_H
