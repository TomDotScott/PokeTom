#ifndef BATTLESTATE_H
#define BATTLESTATE_H

#include "BattleContext.h"
#include "GameContext.h"
#include "MoveComponent.h"
#include "../Engine/IGameState.h"
#include "../Engine/Input/InputMapper.h"
#include "BattleUI/UILayer.h"


class PocketMonsterEntity;

class BattleState : public IGameState
{
public:
	BattleState(GameContext& gameContext, const BattleBeginContext& battleContext);

	void OnEnter() override;
	void OnExit() override;
	void Update(float deltaTime) override;
	void Render(sf::RenderWindow& window) const override;

	GameContext& GetGameContext() const;
	const BattleBeginContext& GetBattleContext() const;

	entity_id_t GetPlayerMonsterEntityID() const;
	entity_id_t GetOpponentMonsterEntityID() const;

private:
	GameContext& m_gameContext;
	BattleBeginContext m_battleContext;

	entity_id_t m_playerMonsterEntityID;
	entity_id_t m_opponentMonsterEntityID;

	bool m_isInBattleLoop;

	eUILayerType m_currentUILayer;

	InputMapper m_inputMapper;

	size_t m_onBattleEndEventID;
	bool m_battleFinished;

	std::array<std::unique_ptr<UILayer>, eUILayerType::COUNT> m_UILayers;

	void OnNavigateButtonPressed(eUILayerNavigateButtons button) const;
	void OnSelectButtonPressed() const;
};

#endif // BATTLESTATE_H
