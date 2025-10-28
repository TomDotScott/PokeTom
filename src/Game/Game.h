#ifndef GAME_H
#define GAME_H
#include <set>
#include <SFML/Graphics/RectangleShape.hpp>
#include "Player.h"
#include "../Engine/Event.h"
#include "../Engine/UI/UiManager.h"
#include "../Engine/ObjectPool.h"
#include "TileLogic.h"
#include "TileRenderer.h"

class Game
{
public:
	Game();
	~Game();

	void Update();

	void Render(sf::RenderWindow& window) const;

private:
	enum eInputIDs
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
	};

	InputMapper m_mapper;
	sf::Vector2f m_playerPosition;

	sf::Vector2f m_cameraPosition;
	sf::Vector2f m_cameraVelocity;

	TileMapData m_mapData;
	TileLogic m_tileLogic;

	TileRenderer m_renderer;

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
};


#endif
