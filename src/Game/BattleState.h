#ifndef BATTLESTATE_H
#define BATTLESTATE_H

#include "BattleContext.h"
#include "GameContext.h"
#include "../Engine/IGameState.h"
#include "../Engine/Input/InputMapper.h"


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
		virtual eUILayer GetNextLayer() const = 0;
		virtual void OnNavigateButtonPressed(eInputs button) = 0;
		virtual void OnSelectButtonPressed() = 0;
		virtual void Update(float deltaTime);
		virtual ~UILayer() = default;

		virtual void OnActivate(const BattleBeginContext& ctx);
		virtual void OnDeactivate();

	protected:
		bool m_finished;
	};

	class OptionSelectLayer : public UILayer
	{
	public:
		OptionSelectLayer();
		eUILayer GetNextLayer() const override;
		void OnNavigateButtonPressed(eInputs button) override;
		void OnSelectButtonPressed() override;
		void OnActivate(const BattleBeginContext& ctx) override;
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

	class RunLayer : public UILayer
	{
	public:
		RunLayer();
		eUILayer GetNextLayer() const override;
		void OnNavigateButtonPressed(eInputs button) override;
		void OnSelectButtonPressed() override;
		void OnActivate(const BattleBeginContext& ctx) override;
		void OnDeactivate() override;

	private:
		BattleEndContext m_endContext;
	};

	std::array<std::unique_ptr<UILayer>, eUILayer::COUNT> m_UILayers;

	void OnNavigateButtonPressed(eInputs button) const;
	void OnSelectButtonPressed() const;
};

#endif // BATTLESTATE_H
