#include "UiButton.h"

#include <iostream>

#include "UiManager.h"
#include "../Asserts.h"

UiButton::UiButton(UiElement* parent) :
	UiElement(eType::Button, parent),
	m_sprite(nullptr),
	m_text(nullptr)
{
	// Text should be drawn on top of it, so let's make the Button have text on top by default
	SetLayer(UiManager::k_topLayer - 1);
}

void UiButton::SetElementPosition(const sf::Vector2f& position)
{
	m_absolutePosition = position;
	SetPosition(position);

	ASSERT(m_sprite != nullptr);
	m_sprite->RecalculatePositionAfterParentMoved();

	if (m_text != nullptr)
	{
		m_text->RecalculatePositionAfterParentMoved();
	}
}

void UiButton::RecalculatePositionAfterParentMoved()
{
	SetElementPosition(m_absolutePosition);
}

void UiButton::OnButtonPressed(const std::function<void()>& callback)
{
	m_pressedEvent.On(callback);
}

void UiButton::OnLeftClickPressed()
{
	m_pressedEvent.Fire();
}

sf::Vector2f UiButton::GetBottomRight() const
{
	return GetPosition() + GetSize();
}

sf::Vector2f UiButton::GetSize() const
{
	return m_sprite->GetSize();
}

UiText* UiButton::GetText() const
{
	return m_text.get();
}

bool UiButton::LoadFromXML(const XmlNode& node)
{
	if (!UiElement::LoadFromXML(node))
	{
		return false;
	}

	const auto* spriteNode = node.Child("Sprite");
	if (spriteNode == nullptr)
	{
		std::cerr << "UiButton::LoadFromXML - No sprite provided!";
		return false;
	}

	m_sprite = std::make_unique<UiSprite>(this);
	m_sprite->LoadFromXML(*spriteNode);

	for (const sf::Drawable* const drawable : m_sprite->GetDrawablesList())
	{
		AddDrawable(drawable);
	}

	if (const auto* textNode = node.Child("Text"); textNode != nullptr)
	{
		m_text = std::make_unique<UiText>();
		m_text->LoadFromXML(*textNode);

		for (const sf::Drawable* const drawable : m_text->GetDrawablesList())
		{
			AddDrawable(drawable);
		}
	}

	return true;
}
