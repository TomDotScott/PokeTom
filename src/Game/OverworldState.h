#ifndef OVERWORLDSTATE_H
#define OVERWORLDSTATE_H
#include <unordered_set>

#include "GameContext.h"
#include "../Engine/IGameState.h"

class OverworldState : public IGameState
{
public:
	OverworldState(GameContext& ctx, hash_type overworldLevel, std::optional<sf::Vector2f> playerPosition, bool healPlayerParty);

	void OnEnter() override;
	void OnExit() override;
	void Update(float deltaTime) override;
	void Render(sf::RenderWindow& window) const override;

private:
	GameContext& m_gameContext;
	sf::FloatRect m_worldBounds;
	sf::FloatRect m_lastCameraRect;
	float m_cameraRebuildThreshold;

	std::optional<hash_type> m_lastEnteredPortalID;

	std::unordered_set<hash_type> m_activeLevelIDs;

	sf::Vector2f m_cameraPosition;
	sf::Vector2f m_cameraVelocity;

	size_t m_onScreenFadedEventID;

	RandomRangeGenerator<float> m_rng;

	void UpdateChunks();
	void UpdateCamera(float deltaTime);
	void CheckForPortals(Entity* player, const Level* currentLevel);
	void CheckForTallGrass(Entity* player, const Level* currentLevel);

	void OnLevelEntered();
	void RespawnPlayerAtPortal(const hash_type& levelName, const hash_type& spawnPointName);
	void RespawnPlayerInWorld(const hash_type& levelName, const sf::Vector2f& position);
};
#endif // OVERWORLDSTATE_H
