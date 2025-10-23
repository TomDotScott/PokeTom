#ifndef MOUSE_H
#define MOUSE_H
#include <array>
#include <SFML/Window/Mouse.hpp>

class Mouse
{
public:
	static Mouse& Get();

	void Update();

	// True the first frame the button is pressed
	bool IsButtonPressed(sf::Mouse::Button button) const;

	// True whilst the buttons are pressed
	bool IsButtonDown(sf::Mouse::Button button) const;

	// True the frame the buttons are released
	bool IsButtonReleased(sf::Mouse::Button button) const;

	void SetRelativeWindow(sf::WindowBase* window);

	sf::Vector2i GetPosition() const;

	// The difference in x since last update
	int GetDX() const;

	// The difference in y since last update
	int GetDY() const;

	// The difference in position since last update
	sf::Vector2i GetDelta() const;

private:
	// True the first frame the button is pressed
	std::array<bool, sf::Mouse::ButtonCount> m_pressedButtons;

	// True whilst the buttons are pressed
	std::array<bool, sf::Mouse::ButtonCount> m_downButtons;

	// True the frame the buttons are released
	std::array<bool, sf::Mouse::ButtonCount> m_releasedButtons;

	sf::Vector2i m_previousPosition;
	sf::Vector2i m_position;

	sf::WindowBase* m_windowReference;

	Mouse();
};


#endif
