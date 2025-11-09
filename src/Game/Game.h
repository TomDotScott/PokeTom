#ifndef GAME_H
#define GAME_H
#include <set>
#include <SFML/Graphics/RectangleShape.hpp>

#include "Level.h"
#include "Player.h"
#include "../Engine/Event.h"
#include "../Engine/UI/UiManager.h"
#include "../Engine/ObjectPool.h"
#include "../Engine/Rendering/ScreenFader.h"
#include "TileLogic.h"
#include "Renderer.h"

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

	Player m_player;
	Level* m_currentLevel;
	Level* m_nextLevel;

	sf::Vector2f m_cameraPosition;
	sf::Vector2f m_cameraVelocity;

	Renderer m_renderer;
	ScreenFader m_screenFader;

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

	void UpdateOverworld(float deltaTime);
	void UpdateScreenFade(float deltaTime);

	void TransitionLevel(const std::filesystem::path& newLevel);

	void ReadyPlayerAndRenderer();
};


#endif
