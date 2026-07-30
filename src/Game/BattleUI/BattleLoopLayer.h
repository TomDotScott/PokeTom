#ifndef BATTLELOOPLAYER_H
#define BATTLELOOPLAYER_H

#include <cstdint>
#include <queue>

#include "UILayer.h"
#include "../BattleContext.h"
#include "../Move.h"

class PocketMonsterEntity;

class BattleLoopLayer final : public UILayer
{
public:
	BattleLoopLayer();
	LayerResult GetLayerResult() const override;
	void Update(float deltaTime) override;
	void OnSelectButtonPressed() override;
	void OnBackButtonPressed() override;
	void OnActivate(const BattleState& state, const LayerResult& prevLayerResult) override;
	void OnDeactivate() override;
	void OnNavigateButtonPressed(eUILayerNavigateButtons button) override;

private:
	bool m_playerSwitchedOut;
	bool m_opponentSwitchedOut;

	// Advance when A is pressed
	struct TextBeat
	{
		std::string m_MessageName;
		std::function<void()> m_OnShow;
		std::optional<std::function<void()>> m_OnDismiss = std::nullopt;
	};

	// Advance when the animation is finished, or when A is pressed
	struct AnimationBeat
	{
		std::function<void()> m_Start;
		std::function<bool()> m_IsComplete;
		std::function<void()> m_FinishAnimation;
	};

	using BattleBeat = std::variant<TextBeat, AnimationBeat>;
	std::queue<BattleBeat> m_battleBeatQueue;

	class HealthbarAnimation
	{
	public:
		enum class eAnimationType
		{
			None,
			Player,
			Opponent
		};

		HealthbarAnimation(eAnimationType type);

		void Start(PocketMonsterEntity* monster, uint16_t monsterHealthBefore);

		void Finish();
		void Update(float deltaTime);

		bool IsPlaying() const;

	private:
		bool m_finished;
		uint16_t m_healthBefore;

		uint16_t m_currentHealth;
		uint16_t m_maxHealth;

		eAnimationType m_type;

		float m_duration;
		float m_currentTime;

		float CalculateFill() const;
		void UpdateFill() const;
	};

	HealthbarAnimation m_playerHealthAnimation;
	HealthbarAnimation m_opponentHealthAnimation;

	BattleEndContext m_endContext;

	void DoTurn(const BattleState& state, PocketMonsterEntity* attacker, uint8_t selectedMoveIdx,
	            PocketMonsterEntity* defender, entity_id_t playerMonsterEntityID);

	void AdvanceBeat();
	void ShowMoveNameText(const PocketMonsterEntity* monster, uint8_t moveIdx) const;
	void ShowFaintText(const PocketMonsterEntity* monster) const;
	void ShowWhiteOutText();

	// TODO: Moves can miss for more than just accuracy (defending monster's evasion stat, attacks failing when used in sequence - i.e. trying to burn a monster that's already burned - and protect/detect. We need to have a case for each of these situations!
	void ShowMissText(const PocketMonsterEntity* monster) const;
	void ShowCriticalHitText() const;
	void ShowEffectivenessText(float moveOutcome) const;
	Move::Outcome UseMove(PocketMonsterEntity* attacker, PocketMonsterEntity* defender, uint8_t moveIdx) const;
	void OnMonsterFainted(const BattleState& state, const PocketMonsterEntity* monster, bool isPlayerMonster);
};
#endif // BATTLELOOPLAYER_H
