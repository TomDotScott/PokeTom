#include "UiText.h"

#include <iostream>

#include "UiManager.h"
#include "../Globals.h"
#include "../CodeGen/Resources.hpp"

UiText::UiText(UiElement* parent) :
	UiElement(eType::Text, parent),
	m_text(DEFAULT_FONT),
	m_alignment(eAlignment::Left)
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

void UiText::SetElementPosition(const sf::Vector2f& position)
{
	m_absolutePosition = position;

	const sf::FloatRect textBounds = m_text.getLocalBounds();

	float originX;
	switch (m_alignment)
	{
		// TODO: Completely redo text alignment as the previous "solution" didn't really work!
	case eAlignment::Left:
	default:
		originX = textBounds.position.x;
		break;
		/*case eAlignment::Right:
			originX = textBounds.position.x + textBounds.size.x;
			break;
		case eAlignment::Centre:
			originX = textBounds.position.x + textBounds.size.x / 2.f;
			break;*/
	}

	float originY = textBounds.position.x + textBounds.size.y / 2.f;
	m_text.setOrigin({ originX, originY });

	const sf::Vector2f finalPosition{ m_absolutePosition + CalculateOffsetFromParents() };
	m_text.setPosition(finalPosition);

	GameObject::SetPosition(finalPosition);
}

void UiText::RecalculatePositionAfterParentMoved()
{
	SetElementPosition(m_absolutePosition);
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

// TODO: Outline
bool UiText::LoadFromXML(const XmlNode& node)
{
	if (!UiElement::LoadFromXML(node))
	{
		return false;
	}

	const auto* fontNode = node.Child("font");
	if (fontNode == nullptr)
	{
		std::cerr << "UiText::LoadFromXML - No <font /> supplied for UiText element " << GetName() << "\n";
		return false;
	}

	// TODO: FontManager?
	const sf::Font* font = UIMANAGER.GetFont(fontNode->m_Content);
	if (!font)
	{
		std::cerr << "UiText::LoadFromXML: Unknown font " << fontNode->m_Content << "\n";
		return false;
	}

	m_text.setFont(*font);

	const auto* stringNode = node.Child("string");
	if (stringNode == nullptr)
	{
		std::cerr << "UiText::LoadFromXML - Warning: No string supplied for UiText element " << GetName() << "\n";
		m_text.setString("");
	}
	else
	{
		m_text.setString(stringNode->m_Content);
	}

	const auto* alignmentNode = node.Child("alignment");
	if (alignmentNode == nullptr)
	{
		std::cerr << "UiText::LoadFromXML - Warning: No alignment supplied for UiText element " << GetName() <<
			". Defaulting to left-aligned text\n";
		SetAlignment(eAlignment::Left);
	}
	else
	{
		const std::string& alignmentText = alignmentNode->m_Content;

		if (alignmentText == "centre")
		{
			SetAlignment(eAlignment::Centre);
		}
		else if (alignmentText == "left")
		{
			SetAlignment(eAlignment::Left);
		}
		else if (alignmentText == "right")
		{
			SetAlignment(eAlignment::Right);
		}
		else
		{
			std::cerr << "UiText::LoadFromXML - Cannot set alignment on element " << GetName() << ". to " <<
				alignmentText << "\n";
			return false;
		}
	}

	const auto* colourNode = node.Child("colour");
	unsigned colour = ~0U;
	if (colourNode != nullptr)
	{
		std::stringstream str;
		str << colourNode->m_Content;
		str >> std::hex >> colour;
	}

	m_text.setFillColor(static_cast<sf::Color>(colour));

	const auto* sizeNode = node.Child("size");
	if (sizeNode != nullptr)
	{
		m_text.setCharacterSize(TRANSFORMED_SCALAR(std::stol(sizeNode->m_Content)));
	}

	AddDrawable(&m_text);
	return true;
}
