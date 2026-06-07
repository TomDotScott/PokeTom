#ifndef BATTLESTATE_H
#define BATTLESTATE_H

#include "BattleContext.h"
#include "GameContext.h"
#include "MoveComponent.h"
#include "../Engine/IGameState.h"
#include "../Engine/Input/InputMapper.h"


class PocketMonsterEntity;

class BattleState : public IGameState
{
public:
	BattleState(GameContext& gameContext, const BattleBeginContext& battleContext);

	void OnEnter() override;
	void OnExit() override;
	void Update(float deltaTime) override;
	void Render(sf::RenderWindow& window) const override;

private:
	GameContext& m_gameContext;
	BattleBeginContext m_battleContext;

	entity_id_t m_playerMonsterEntityID;
	entity_id_t m_opponentMonsterEntityID;

	enum eUILayer : uint8_t
	{
		OptionSelect,
		MoveSelect,
		MonsterSelect,
		ItemSelect,
		QuitBattle,
		COUNT
	};

	eUILayer m_currentUILayer;

	enum eInputs : uint8_t
	{
		SELECT = 1 << 0,
		UP = 1 << 1,
		DOWN = 1 << 2,
		LEFT = 1 << 3,
		RIGHT = 1 << 4,
	};

	InputMapper m_inputMapper;

	class UILayer
	{
	public:
		UILayer();
		bool IsFinished() const;
		virtual void OnNavigateButtonPressed(eInputs button) = 0;
		virtual void OnSelectButtonPressed() = 0;
		virtual void Update(float deltaTime);
		virtual ~UILayer() = default;

		virtual void OnActivate(const BattleState& state);
		virtual void OnDeactivate();

		struct LayerResult
		{
			eUILayer m_NextLayer;
			std::optional<uint32_t> m_ChosenMoveID;
		};

		virtual LayerResult GetLayerResult() const = 0;

	protected:
		bool m_finished;
	};

	class OptionSelectLayer final : public UILayer
	{
	public:
		OptionSelectLayer();
		LayerResult GetLayerResult() const override;
		void OnNavigateButtonPressed(eInputs button) override;
		void OnSelectButtonPressed() override;
		void OnActivate(const BattleState& state) override;
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

	class FightLayer final : public UILayer
	{
	public:
		FightLayer();
		LayerResult GetLayerResult() const override;
		void OnNavigateButtonPressed(eInputs button) override;
		void OnSelectButtonPressed() override;
		void OnActivate(const BattleState& state) override;
		void OnDeactivate() override;

	private:
		PocketMonsterEntity* m_playerMonster;
		PocketMonsterEntity* m_opponentMonster;

		std::array<bool, MOVE_COUNT> m_validMoves;
	};

	class RunLayer final : public UILayer
	{
	public:
		RunLayer();
		LayerResult GetLayerResult() const override;
		void OnNavigateButtonPressed(eInputs button) override;
		void OnSelectButtonPressed() override;
		void OnActivate(const BattleState& state) override;
		void OnDeactivate() override;

	private:
		BattleEndContext m_endContext;
	};

	std::array<std::unique_ptr<UILayer>, eUILayer::COUNT> m_UILayers;

	void OnNavigateButtonPressed(eInputs button) const;
	void OnSelectButtonPressed() const;
};

#endif // BATTLESTATE_H
