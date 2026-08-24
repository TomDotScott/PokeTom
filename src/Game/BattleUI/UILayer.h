#ifndef UILAYER_H
#define UILAYER_H

#include <cstdint>
#include <optional>

class BattleState;

enum eUILayerType : uint8_t
{
	OptionSelect,
	MoveSelect,
	MonsterSelect,
	ItemSelect,
	BattleLoop,
	QuitBattle,
	COUNT
};

enum eUILayerNavigateButtons : uint8_t
{
	SELECT = 1 << 0,
	BACK = 1 << 1,
	UP = 1 << 2,
	DOWN = 1 << 3,
	LEFT = 1 << 4,
	RIGHT = 1 << 5,
	MORE_INFO = 1 << 6
};

constexpr const char* BATTLE_PANEL_NAME = "BATTLE_HUD_PANEL";
constexpr const char* OPTIONS_PANEL_NAME = "BATTLE_OPTIONS";
constexpr const char* MOVES_PANEL_NAME = "MOVE_OPTIONS";
constexpr const char* BATTLE_TEXT_NAME = "TEXT_BOX_TEXT";



class UILayer
{
public:
	UILayer();
	bool IsFinished() const;
	virtual void OnNavigateButtonPressed(eUILayerNavigateButtons button) = 0;
	virtual void OnSelectButtonPressed() = 0;
	virtual void OnBackButtonPressed() = 0;
	virtual void OnMoreInfoButtonPressed() = 0;
	virtual void Update(float deltaTime);
	virtual ~UILayer() = default;

	struct LayerResult
	{
		eUILayerType m_NextLayer;
		std::optional<uint8_t> m_ChosenMoveIndex; // 0-3
	};

	virtual LayerResult GetLayerResult() const = 0;

	virtual void OnActivate(const BattleState& state, const LayerResult& prevLayerResult);
	virtual void OnDeactivate();

protected:
	bool m_finished;
};

#endif // UILAYER_H
