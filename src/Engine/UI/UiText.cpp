#include "UiText.h"

#include "UiManager.h"
#include "../Globals.h"

UiText::UiText(UiElement* parent) :
	UiElement(eType::Text),
	m_text(DEFAULT_FONT),
	m_alignment(eAlignment::Left),
	m_parent(parent)
{
	// All text drawn on top
	// TODO: Make this more sophisticated... I need to be able to support arbitrary placement of elements in the XML otherwise we are at the mercy of std::hash!
	SetLayer(eLayer::FOREGROUND);
}

const char* UiText::GetText() const
{
	return m_text.getString().toAnsiString().c_str();
}

void UiText::SetTextSize(const unsigned size)
{
	m_text.setCharacterSize(size);
}

void UiText::SetPosition(const sf::Vector2f& position)
{
	GameObject::SetPosition(position);

	if (m_parent)
	{
		CalculateAlignmentBounds();
	}
	else
	{
		// No parent, fallback to normal position with origin centered horizontally and vertically
		const sf::FloatRect textBounds = m_text.getLocalBounds();

		float originX;
		switch (m_alignment)
		{
		case eAlignment::Left:
			originX = textBounds.position.x;
			break;
		case eAlignment::Right:
			originX = textBounds.position.x + textBounds.size.x;
			break;
		case eAlignment::Centre:
		default:
			originX = textBounds.position.x + textBounds.size.x / 2.f;
			break;
		}

		float originY = textBounds.position.x + textBounds.size.y / 2.f;
		m_text.setOrigin({ originX, originY });
		m_text.setPosition(position);
	}
}

UiText::eAlignment UiText::GetAlignment() const
{
	return m_alignment;
}

void UiText::SetAlignment(const eAlignment alignment)
{
	m_alignment = alignment;
}

sf::Vector2f UiText::GetSize() const
{
	return m_text.getGlobalBounds().size;
}

void UiText::CalculateAlignmentBounds()
{
	const sf::FloatRect textBounds = m_text.getLocalBounds();
	if (!m_parent)
	{
		// Fallback: no parent, just use position and center origin
		m_text.setOrigin({ textBounds.position.x + textBounds.size.x / 2.f,
			textBounds.position.y + textBounds.size.y / 2.f });
		return;
	}

	const sf::Vector2f parentPos = m_parent->GetPosition();
	const sf::Vector2f parentSize = m_parent->GetSize();

	float originX;
	float posX;

	switch (m_alignment)
	{

	case eAlignment::Right:
		originX = textBounds.position.x + textBounds.size.x;
		posX = parentPos.x + parentSize.x;
		break;
	case eAlignment::Centre:
		originX = textBounds.position.x + textBounds.size.x / 2.f;
		posX = parentPos.x + parentSize.x / 2.f;
		break;
	case eAlignment::Left:
	default:
		originX = textBounds.position.x;
		posX = parentPos.x;
		break;
	}

	// Vertically center always (assuming that's what you want)
	float originY = textBounds.position.y + textBounds.size.y / 2.f;
	float posY = parentPos.y + parentSize.y / 2.f;

	m_text.setOrigin({ originX, originY });
	m_text.setPosition({ posX, posY });
}

bool UiText::ParseEndElement(hoxml_context_t*& context)
{
	if (!context)
	{
		return false;
	}

	if (context->tag && strcmp("Text", context->tag) == 0)
	{
		if (!m_string.empty())
		{
			printf(" Closing tag found and no text was assigned to %s!\n", GetName().c_str());
			return false;
		}

		m_text.setPosition(m_position);

		AddDrawable(&m_text);

		// When we have finished assigning the sprite, we can return true!
		return true;
	}

	if (context->content == nullptr || OnlyWhitespace(context->content))
	{
		return false;
	}

	if (strcmp("string", context->tag) == 0)
	{
		m_text.setString(context->content);
		return false;
	}

	if (strcmp("colour", context->tag) == 0)
	{
		char* p;
		const unsigned long n = strtoul(context->content, &p, 16);
		if (*p != 0)
		{
			printf(" ERROR: BAD COLOUR INPUT %s\n", context->content);
			return false;
		}

		m_text.setFillColor(static_cast<sf::Color>(n));

		return false;
	}

	if (strcmp("outline", context->tag) == 0)
	{
		// TODO...
	}

	if (strcmp("size", context->tag) == 0)
	{
		m_text.setCharacterSize(TRANSFORMED_SCALAR(std::stol(context->content)));
		return false;
	}

	if (strcmp("alignment", context->tag) == 0)
	{
		if (strcmp(context->content, "centre") == 0)
		{
			SetAlignment(eAlignment::Centre);
		}
		else if (strcmp(context->content, "left") == 0)
		{
			SetAlignment(eAlignment::Left);
		}
		else if (strcmp(context->content, "right") == 0)
		{
			SetAlignment(eAlignment::Right);
		}
		return false;
	}

	if (strcmp("font", context->tag) == 0)
	{
		// TODO: FontManager?
		const sf::Font* font = UIMANAGER.GetFont(context->content);
		if (!font)
		{
			printf(" UiText: Unknown font %s on line %u\n", context->content, context->line);
			return false;
		}

		m_text.setFont(*font);
		return false;
	}

	return UiElement::ParseEndElement(context);
}
