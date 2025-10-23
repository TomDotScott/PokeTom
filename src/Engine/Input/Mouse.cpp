#include "Mouse.h"

Mouse::Mouse() :
	m_pressedButtons(),
	m_downButtons(),
	m_releasedButtons(),
	m_windowReference(nullptr)
{
}

Mouse& Mouse::Get()
{
	static Mouse* instance = nullptr;
	if (instance == nullptr)
	{
		instance = new Mouse();
	}

	return *instance;
}

void Mouse::Update()
{
	std::array<bool, sf::Mouse::ButtonCount> buttonsPressedThisFrame{};

	// Get the buttons that changed this frame
	for (unsigned i = 0; i < sf::Mouse::ButtonCount; i++)
	{
		buttonsPressedThisFrame[i] = sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(i));

		// Is it newly pressed?
		m_pressedButtons[i] = buttonsPressedThisFrame[i] && !m_downButtons[i];

		// Is it newly released?
		m_releasedButtons[i] = !buttonsPressedThisFrame[i] && m_downButtons[i];

		// Store the state
		m_downButtons[i] = buttonsPressedThisFrame[i];
	}

	m_previousPosition = m_position;

	if (m_windowReference != nullptr)
	{
		m_position = sf::Mouse::getPosition(*m_windowReference);
	}
	else
	{
		m_position = sf::Mouse::getPosition();
	}
}

void Mouse::SetRelativeWindow(sf::WindowBase* window)
{
	m_windowReference = window;
}

sf::Vector2i Mouse::GetPosition() const
{
	return m_position;
}

int Mouse::GetDX() const
{
	return (m_position - m_previousPosition).x;
}

int Mouse::GetDY() const
{
	return (m_position - m_previousPosition).y;
}

sf::Vector2i Mouse::GetDelta() const
{
	return (m_position - m_previousPosition);
}

bool Mouse::IsButtonPressed(const sf::Mouse::Button button) const
{
	return m_pressedButtons[static_cast<int>(button)];
}

bool Mouse::IsButtonDown(const sf::Mouse::Button button) const
{
	return m_downButtons[static_cast<int>(button)];
}

bool Mouse::IsButtonReleased(const sf::Mouse::Button button) const
{
	return m_releasedButtons[static_cast<int>(button)];
}
