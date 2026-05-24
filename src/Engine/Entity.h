#ifndef ENTITY_H
#define ENTITY_H
#include "Gameobject.h"
#include "IUpdateable.h"
#include "Orientation.h"
#include "Animation/AnimationPlayer.h"


class Entity : public GameObject, public IUpdateable
{
public:
	Entity();
	Entity(const sf::Vector2f& position, const sf::Vector2f& size);

	~Entity() override;

	void Update(float deltaTime) override;

	void SetPosition(const sf::Vector2f& position);

	template<typename T>
	T* GetComponent()
	{
		for (auto& c : m_components)
		{
			if (T* cast = dynamic_cast<T*>(c.get()))
			{
				return cast;
			}
		}

		return nullptr;
	}

	template<typename T>
	const T* GetComponent() const
	{
		for (const auto& c : m_components)
		{
			if (const T* cast = dynamic_cast<const T*>(c.get()))
			{
				return cast;
			}
		}

		return nullptr;
	}

	template<typename T, typename ...Args>
	T& AddComponent(Args&&... args)
	{
		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		T& ref = *component;
		m_components.emplace_back(std::move(component));
		return ref;
	}

	void OnActivate() override;
	void OnDeactivate() override;
	void OnPlayerInteractPressed() override;

	virtual void OnEntityDestroyed();

	sf::Vector2f GetSize();
	const sf::Vector2f& GetSize() const;

	sf::Vector2f GetScale();
	const sf::Vector2f& GetScale() const;

	void SetScale(const sf::Vector2f& scale);

private:
	std::vector<std::unique_ptr<IUpdateable>> m_components;
	sf::Vector2f m_size;
	sf::Vector2f m_scale;

	void OnEntityActivate();
	void OnEntityDeactivate();
};
#endif
