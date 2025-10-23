#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <array>
#include <SFML/Window/Keyboard.hpp>

class Keyboard
{
public:
	static Keyboard& Get();

	void Update();

	// True the first frame the button is pressed
	bool IsButtonPressed(sf::Keyboard::Key key) const;

	// True whilst the buttons are pressed
	bool IsButtonDown(sf::Keyboard::Key key) const;

	// True the frame the buttons are released
	bool IsButtonReleased(sf::Keyboard::Key key) const;

private:
	// True the first frame the button is pressed
	std::array<bool, sf::Keyboard::KeyCount> m_pressedButtons;

	// True whilst the buttons are pressed
	std::array<bool, sf::Keyboard::KeyCount> m_downButtons;

	// True the frame the buttons are released
	std::array<bool, sf::Keyboard::KeyCount> m_releasedButtons;

	Keyboard();
};


#endif
