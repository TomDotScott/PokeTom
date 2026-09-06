#include "Entity.h"

#include <SFML/Graphics/Shader.hpp>

#include "Asserts.h"
#include "GridMovementComponent.h"
#include "Animation/AnimationComponent.h"
#include "../Engine/CodeGen/Resources.hpp"

Entity::Entity() : Entity(sf::Vector2f{}, sf::Vector2f{})
{
}

Entity::Entity(const sf::Vector2f& position, const sf::Vector2f& size) :
	GameObject(position),
	IUpdateable(),
	m_size(size)
{
	SetScale({ 2.f, 2.f });
}

Entity::~Entity()
{
	Entity::OnEntityDestroyed();
}

void Entity::Update(const float deltaTime)
{
	for (const auto& c : m_components)
	{
		c->Update(deltaTime);
	}

	// Sync position from movement component if one exists
	if (const GridMovementComponent* movement = GetComponent<GridMovementComponent>())
	{
		SetPosition(movement->GetWorldPosition());
	}
}

void Entity::SetPosition(const sf::Vector2f& position)
{
	GameObject::SetPosition(position);

	if (GridMovementComponent* movement = GetComponent<GridMovementComponent>())
	{
		movement->SetWorldPosition(position);
	}
}

void Entity::OnActivate()
{
	GameObject::OnActivate();

	OnEntityActivate();
}

void Entity::OnDeactivate()
{
	GameObject::OnDeactivate();

	OnEntityDeactivate();
}

void Entity::OnEntityDestroyed()
{
	IUpdateable::OnDestroyed();

	for (auto& c : m_components)
	{
		c->OnDestroyed();
		c.reset(nullptr);
	}

	m_components.clear();
}

sf::Vector2f Entity::GetSize()
{
	return m_size;
}

const sf::Vector2f& Entity::GetSize() const
{
	return m_size;
}

void Entity::SetOffsetPosition(const sf::Vector2f& position)
{
	m_animationOffset.setPosition(position);
}

sf::Vector2f Entity::GetOffsetPosition() const
{
	return m_animationOffset.getPosition();
}

void Entity::SetOffsetRotation(const sf::Angle& angle)
{
	m_animationOffset.setRotation(angle);
}

sf::Angle Entity::GetOffsetRotation() const
{
	return m_animationOffset.getRotation();
}

void Entity::SetOffsetScale(const sf::Vector2f& scale)
{
	m_animationOffset.setScale(scale);
}

sf::Vector2f Entity::GetOffsetScale() const
{
	return m_animationOffset.getScale();
}

bool Entity::LoadShader(const std::string_view& shaderResourceName, const sf::Shader::Type type)
{
	ASSERT(!m_shaders.contains(shaderResourceName));

	m_shaders[shaderResourceName] = sf::Shader();

	return m_shaders.at(shaderResourceName).loadFromFile(GET_SHADER_PATH(shaderResourceName), type);
}

void Entity::SetCurrentShader(const std::string_view& shaderResourceName)
{
	m_currentShader = shaderResourceName;
}

sf::Shader* Entity::GetShader(const std::string_view& shaderResourceName)
{
	if (m_shaders.contains(shaderResourceName))
	{
		return &m_shaders.at(shaderResourceName);
	}

	return nullptr;
}

sf::Shader* Entity::GetCurrentShader()
{
	if (m_currentShader.empty())
	{
		return nullptr;
	}

	return GetShader(m_currentShader);
}

void Entity::OnPlayerInteractPressed()
{
	IUpdateable::OnPlayerInteractPressed();

	for (const auto& c : m_components)
	{
		c->OnPlayerInteractPressed();
	}
}

void Entity::OnEntityActivate()
{
	IUpdateable::OnActivate();

	for (const auto& c : m_components)
	{
		c->OnActivate();
	}
}

void Entity::OnEntityDeactivate()
{
	IUpdateable::OnDeactivate();

	for (const auto& c : m_components)
	{
		c->OnDeactivate();
	}
}
