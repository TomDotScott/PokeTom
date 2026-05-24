#ifndef BATTLESTATE_H
#define BATTLESTATE_H

#include "BattleContext.h"
#include "GameContext.h"
#include "../Engine/IGameState.h"
#include "../Engine/Input/InputMapper.h"


class BattleState : public IGameState
{
public:
	BattleState(GameContext& gameContext, BattleBeginContext battleContext);

	void OnEnter() override;
	void OnExit() override;
	void Update(float deltaTime) override;
	void Render(sf::RenderWindow& window) const override;

private:
	GameContext& m_gameContext;
	BattleBeginContext m_battleContext;

	entity_id_t m_playerMonsterEntityID;
	entity_id_t m_opponentMonsterEntityID;

	enum class eSelectedOption : uint8_t
	{
		FIGHT,
		MONSTERS,
		BAG,
		RUN
	};
	eSelectedOption m_selectedOption;

	enum eInputs : uint8_t
	{
		SELECT = 1 << 0,
		UP = 1 << 1,
		DOWN = 1 << 2,
		LEFT = 1 << 3,
		RIGHT = 1 << 4
	};
	InputMapper m_inputMapper;

	void OnNavigateButtonPressed(eInputs button);
	void OnSelectedOptionChanged(eSelectedOption newOption);
};

#endif // BATTLESTATE_H
