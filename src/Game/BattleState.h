#ifndef BATTLESTATE_H
#define BATTLESTATE_H

#include "BattleContext.h"
#include "GameContext.h"
#include "../Engine/IGameState.h"
#include "../Engine/Input/InputMapper.h"


class BattleState : public IGameState
{
public:
	BattleState(GameContext& gameContext, const BattleContext& battleContext);

	void OnEnter() override;
	void OnExit() override;
	void Update(float deltaTime) override;
	void Render(sf::RenderWindow& window) const override;

private:
	GameContext& m_gameContext;
	BattleContext m_battleContext;

	enum eInputs : uint8_t
	{
		SELECT_MOVE = 1,
	};
	InputMapper m_inputMapper;
};

#endif // BATTLESTATE_H
