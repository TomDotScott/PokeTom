#ifndef GAME_H
#define GAME_H
#include <set>
#include <SFML/Graphics/RectangleShape.hpp>

#include "Level.h"
#include "Player.h"
#include "Renderer.h"
#include "TileLogic.h"
#include "WorldDefinition.h"
#include "../Engine/Event.h"
#include "../Engine/ObjectPool.h"
#include "../Engine/ScriptedEntity.h"
#include "../Engine/Rendering/ScreenFader.h"
#include "../Engine/UI/UiManager.h"

class Game final : public Updateable
{
public:
	Game();
	~Game() override;

	void Update(float deltaTime) override;

	void Render(sf::RenderWindow& window) const;

private:
	enum class eGameState
	{
		FadingScreen,
		Overworld
	} m_state;

	WorldDefinition m_world;
	Player m_player;
	ScriptedEntity m_man;

	std::vector<std::shared_ptr<Entity>> m_entities;

	const PortalTrigger* m_lastEnteredPortal;

	sf::Vector2f m_cameraPosition;
	sf::Vector2f m_cameraVelocity;

	Renderer m_renderer;
	ScreenFader m_screenFader;

	sf::FloatRect m_worldBounds;
	sf::FloatRect m_lastCameraRect;
	float m_cameraRebuildThreshold;

#if !BUILD_MASTER
	template<typename... Args>
	static void DrawText(sf::RenderWindow& window, const sf::Vector2f& position, const int size, const char* fmt, Args... args)
	{
		if (auto* debugUi = UIMANAGER.GetUiText("DEBUG_TEXT"))
		{
			debugUi->SetPosition(position);
			debugUi->SetTextSize(size);
			debugUi->SetText(fmt, args...);

			UIMANAGER.DrawDebugText(window);
		}
	}
#endif

	void UpdateChunks();
	void UpdateOverworld(float deltaTime);
	void UpdateCamera(float deltaTime);
	void CheckForPortals();

	void UpdateScreenFade(float deltaTime);

	void RespawnPlayer(const std::string& levelName, const std::string& spawnPointName, bool shouldSetPlayerPosition);

	void TransitionLevel();
	void OnTransitionEnd();
};


#endif
