#include "UiButton.h"

#include "UiManager.h"
#include "../Globals.h"
#include "../TextureManager.h"
#include "../Input/Mouse.h"

UiButton::UiButton() :
	UiElement(eType::Button),
	m_sprite(nullptr)
{
	// Always render on top
	SetLayer(eLayer::FOREGROUND);
}

void UiButton::SetPosition(const sf::Vector2f& position)
{
	UiElement::SetPosition(position);

	m_sprite->SetPosition(position);

	if (m_offsetText.m_text) {
		sf::Vector2f anchor = m_position;

		m_offsetText.m_text->SetPosition(anchor + m_offsetText.m_offset);
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
	return m_offsetText.m_text;
}

bool UiButton::ParseBeginElement(hoxml_context_t*& context)
{
	auto [xmlText, xmlLength] = UIMANAGER.GetLastXmlDetails();

	if (strcmp("Sprite", context->tag) == 0)
	{
		m_sprite = new UiSprite();
		if (!m_sprite->Load(context, xmlText, xmlLength))
		{
			printf(" UiPanel: Error loading sprite on line %u\n", context->line);
			return true;
		}
	}
	if (strcmp("Text", context->tag) == 0)
	{
		m_offsetText.m_text = new UiText(this);

		if (!m_offsetText.m_text->Load(context, xmlText, xmlLength))
		{
			printf(" UiPanel: Error loading Text on line %u\n", context->line);
			return true;
		}

		m_offsetText.m_offset = m_offsetText.m_text->GetPosition();
	}

	return UiElement::ParseBeginElement(context);
}

bool UiButton::ParseEndElement(hoxml_context_t*& context)
{
	if (!context)
	{
		return false;
	}

	if (context->tag && strcmp("Button", context->tag) == 0)
	{
		if (!m_sprite)
		{
			printf(" Closing tag found and no sprite was assigned to %s!\n", GetName().c_str());
			return false;
		}

		for (const sf::Drawable* const drawable : m_sprite->GetDrawablesList())
		{
			AddDrawable(drawable);
		}

		if (m_offsetText.m_text)
		{
			for (const sf::Drawable* const drawable : m_offsetText.m_text->GetDrawablesList())
			{
				AddDrawable(drawable);
			}
		}

		SetPosition(m_position);

		// When we have finished assigning the sprite, we can return true!
		return true;
	}

	if (context->content == nullptr || OnlyWhitespace(context->content))
	{
		return false;
	}

	return UiElement::ParseEndElement(context);
}
