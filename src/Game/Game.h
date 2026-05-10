#ifndef GAME_H
#define GAME_H
#include <SFML/Graphics/RectangleShape.hpp>
#include <sol/sol.hpp>

#include "GameContext.h"
#include "LuaRegistry.h"
#include "Player.h"
#include "../Engine/IGameState.h"
#include "../Engine/Rendering/ScreenFader.h"
#include "../Engine/UI/UiManager.h"

class Game final : public IUpdateable
{
public:
	Game(sol::state& lua);
	~Game() override;

	void Update(float deltaTime) override;

	void Render(sf::RenderWindow& window) const;

private:
	GameContext m_context;

	std::unique_ptr<IGameState> m_currentState;
	std::unique_ptr<IGameState> m_pendingState;

	LuaRegistry m_luaBindings;
	ScreenFader m_screenFader;

#if !BUILD_MASTER
	template<typename... Args>
	static void DrawText(sf::RenderWindow& window, const sf::Vector2f& position, const int size, const char* fmt, Args... args)
	{
		if (auto* debugUi = UIMANAGER.GetUiText("DEBUG_TEXT"))
		{
			debugUi->SetElementPosition(position);
			debugUi->SetTextSize(size);
			debugUi->SetText(fmt, args...);

			UIMANAGER.DrawDebugText(window);
		}
	}
#endif

	void UpdateScreenFade(float deltaTime);
	void RequestTransition(std::unique_ptr<IGameState> next);
};


#endif
