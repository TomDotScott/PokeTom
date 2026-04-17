#include "UiButton.h"

#include "UiManager.h"

UiButton::UiButton() :
	UiElement(eType::Button),
	m_sprite(nullptr)
{
	// Always render on top
	SetLayer(eLayer::FOREGROUND);
}

void UiButton::SetElementPosition(const sf::Vector2f& position)
{
	SetPosition(position);

	m_sprite->SetElementPosition(position);

	if (m_offsetText.m_text)
	{
		const sf::Vector2f anchor = m_position;

		m_offsetText.m_text->SetElementPosition(anchor + m_offsetText.m_offset);
	}
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
	return m_offsetText.m_text.get();
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
		return false;
	}

	m_sprite = std::make_unique<UiSprite>();
	m_sprite->LoadFromXML(*spriteNode);

	for (const sf::Drawable* const drawable : m_sprite->GetDrawablesList())
	{
		AddDrawable(drawable);
	}

	if (const auto* textNode = node.Child("Text"); textNode != nullptr)
	{
		m_offsetText.m_text = std::make_unique<UiText>();
		m_offsetText.m_text->LoadFromXML(*textNode);

		m_offsetText.m_offset = m_offsetText.m_text->GetPosition();

		for (const sf::Drawable* const drawable : m_offsetText.m_text->GetDrawablesList())
		{
			AddDrawable(drawable);
		}
	}

	return true;
}
