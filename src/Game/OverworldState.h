#ifndef OVERWORLDSTATE_H
#define OVERWORLDSTATE_H
#include "GameContext.h"
#include "../Engine/IGameState.h"

class OverworldState : public IGameState
{
public:
	OverworldState(GameContext& ctx);

	void OnEnter() override;
	void OnExit() override;
	void Update(float deltaTime) override;
	void Render(sf::RenderWindow& window) const override;

private:
	GameContext& m_ctx;
	sf::FloatRect m_worldBounds;
	sf::FloatRect m_lastCameraRect;
	float m_cameraRebuildThreshold;

	std::optional<hash_type> m_lastEnteredPortalID;

	sf::Vector2f m_cameraPosition;
	sf::Vector2f m_cameraVelocity;

	void UpdateChunks();
	void UpdateCamera(float deltaTime);
	void CheckForPortals(Entity* player, const Level* currentLevel);
	void OnLevelEntered();
	void RespawnPlayer(const hash_type& levelName, const hash_type& spawnPointName, bool shouldSetPlayerPosition);
};
#endif // OVERWORLDSTATE_H
