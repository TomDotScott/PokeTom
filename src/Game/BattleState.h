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

	enum eInputs : uint8_t
	{
		SELECT_MOVE = 1,
	};
	InputMapper m_inputMapper;
};

#endif // BATTLESTATE_H
