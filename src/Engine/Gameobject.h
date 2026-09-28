#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>

class GameObject
{
public:
	explicit GameObject(sf::Vector2f position = { 0.f, 0.f });

	virtual ~GameObject() = default;

	virtual void OnActivate();

	virtual void OnDeactivate();

	sf::Vector2f GetPosition() const;
	void SetPosition(const sf::Vector2f& position);
	void SetPosition(float x, float y);

	sf::Angle GetRotation() const;
	void SetRotation(const sf::Angle& angle);

	sf::Vector2f GetScale() const;
	void SetScale(const sf::Vector2f& scale);

	uint32_t GetID() const;

	bool IsActive() const { return m_isActive; }

protected:
	sf::Transformable m_transformable;

private:
	bool m_isActive;
	uint32_t m_id;
};

#endif
