#ifndef BATTLELOOPLAYER_H
#define BATTLELOOPLAYER_H

#include <cstdint>
#include <queue>

#include "HealthbarAnimation.h"
#include "UILayer.h"
#include "../BattleContext.h"
#include "../Move.h"
#include "../Monsters/MonsterAnimation.h"

class BattleLoopLayer final : public UILayer
{
public:
	BattleLoopLayer();
	LayerResult GetLayerResult() const override;
	void Update(float deltaTime) override;
	void OnSelectButtonPressed() override;
	void OnBackButtonPressed() override;
	void OnMoreInfoButtonPressed() override;
	void OnActivate(const BattleState& state, const LayerResult& prevLayerResult) override;
	void OnDeactivate() override;
	void OnNavigateButtonPressed(eUILayerNavigateButtons button) override;

private:
	bool m_playerSwitchedOut;
	bool m_opponentSwitchedOut;

	UiText* m_textBoxText;

	// Advance when A is pressed
	struct TextBeat
	{
		std::string m_BeatName;
		std::function<void()> m_OnShow;
		std::optional<std::function<void()>> m_OnDismiss = std::nullopt;
	};

	// Advance when the animation is finished, or when A is pressed
	struct AnimationBeat
	{
		std::string m_BeatName;
		std::function<void()> m_Start;
		std::function<void(float)> m_Update;
		std::function<bool()> m_IsComplete;
		std::function<void()> m_FinishAnimation;
	};

	using BattleBeat = std::variant<TextBeat, AnimationBeat>;
	std::queue<BattleBeat> m_battleBeatQueue;

	HealthbarAnimation m_playerHealthAnimation;
	AnimatedProgressBar<monster_xp_t> m_playerExperienceBar;

	HealthbarAnimation m_opponentHealthAnimation;

	BattleEndContext m_endContext;

	void DoTurn(const BattleState& state, PocketMonsterEntity* attacker, uint8_t selectedMoveIdx,
	            PocketMonsterEntity* defender, entity_id_t playerMonsterEntityID);

	void AdvanceBeat();
	void ShowMoveNameText(const PocketMonsterEntity* monster, uint8_t moveIdx) const;
	void ShowFaintText(const PocketMonsterEntity* monster) const;
	void ShowWhiteOutText() const;

	// TODO: Moves can miss for more than just accuracy (defending monster's evasion stat, attacks failing when used in sequence - i.e. trying to burn a monster that's already burned - and protect/detect. We need to have a case for each of these situations!
	void ShowMissText(const PocketMonsterEntity* monster) const;
	void ShowCriticalHitText() const;
	void ShowEffectivenessText(float moveOutcome) const;
	void ShowStatChangeText(const PocketMonsterEntity* monster, StatChange::StatStage statChangeInfo, bool succeeded) const;
	void ShowExperienceText(monster_xp_t xpGained) const;
	void ShowLevelUpText(const PocketMonsterEntity* monster, uint8_t level) const;

	Move::Outcome UseMove(PocketMonsterEntity* attacker, PocketMonsterEntity* defender, uint8_t moveIdx) const;
	void UpdateExperienceBar(monster_xp_t xpGained, PocketMonsterEntity* playerMonster, const PocketMonsterEntity* opponentMonster);
	void OnMonsterFainted(const BattleState& state, const PocketMonsterEntity* monster, bool isPlayerMonster);

	void RefreshPlayerUI(PocketMonsterEntity* playerMonster, uint8_t currentLevel);
};
#endif // BATTLELOOPLAYER_H
