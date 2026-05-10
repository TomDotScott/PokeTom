#ifndef IGAMESTATE_H
#define IGAMESTATE_H
#include <SFML/Graphics/RenderWindow.hpp>

class IGameState
{
public:
	virtual ~IGameState() = default;
	virtual void OnEnter() = 0;
	virtual void OnExit() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render(sf::RenderWindow& window) const = 0;
};

#endif // IGAMESTATE_H
