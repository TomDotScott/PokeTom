#ifndef ENTITY_H
#define ENTITY_H
#include <SFML/Graphics/Shader.hpp>

#include "Asserts.h"
#include "Gameobject.h"
#include "IUpdateable.h"
#include "Animation/AnimationPlayer.h"

using entity_id_t = uint32_t;

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

	template<typename T>
	bool HasComponent() const
	{
		return GetComponent<T>() != nullptr;
	}

	void OnActivate() override;
	void OnDeactivate() override;
	void OnPlayerInteractPressed() override;

	virtual void OnEntityDestroyed();

	sf::Vector2f GetSize();
	const sf::Vector2f& GetSize() const;

	void SetOffsetPosition(const sf::Vector2f& position);
	sf::Vector2f GetOffsetPosition() const;

	void SetOffsetRotation(const sf::Angle& angle);
	sf::Angle GetOffsetRotation() const;

	void SetOffsetScale(const sf::Vector2f& scale);
	sf::Vector2f GetOffsetScale() const;

	bool LoadShader(const std::string_view& shaderResourceName, sf::Shader::Type) const;
	sf::Shader* GetShader() const;
	template<typename T>
	void SetShaderVariable(const std::string& uniformName, T value)
	{
		ASSERT(m_shaders.has_value());
		m_shaders.value().setUniform(uniformName, value);
	}

private:
	std::vector<std::unique_ptr<IUpdateable>> m_components;
	sf::Vector2f m_size;

	sf::Transformable m_animationOffset;

	// TODO: Have multiple shader passes? Idk, I'm not a graphics programmer
	mutable std::optional<sf::Shader> m_shaders;

	void OnEntityActivate();
	void OnEntityDeactivate();
};
#endif
