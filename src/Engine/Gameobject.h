#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <functional>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

class Updatable
{
public:
	virtual ~Updatable() = default;

	virtual void Update() = 0;
};

class GameObject
{
public:
	explicit GameObject(sf::Vector2f position = { 0.f, 0.f });

	virtual ~GameObject() = default;

	void OnActivate();

	void OnDeactivate();

	const sf::Vector2f& GetPosition() const { return m_position; }
	sf::Vector2f GetPosition() { return m_position; }
	virtual void SetPosition(const sf::Vector2f& position);

	uint64_t GetID() const;

	bool IsActive() const { return m_isActive; }

protected:
	sf::Vector2f m_position;

private:
	bool m_isActive;
	uint64_t m_id;
};

#endif
