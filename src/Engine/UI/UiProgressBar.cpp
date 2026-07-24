#include "UiProgressBar.h"


UiProgressBar::UiProgressBar(UiElement* parent) :
	UiElement(eType::ProgressBar, parent)
{
}

sf::Vector2f UiProgressBar::GetSize() const
{
	return m_background.getGlobalBounds().size;
}

void UiProgressBar::RecalculatePositionAfterParentMoved()
{
}

void UiProgressBar::OnActivate()
{
	UiElement::OnActivate();
}

void UiProgressBar::OnDeactivate()
{
	UiElement::OnDeactivate();
}

void UiProgressBar::SetElementPosition(const sf::Vector2f& position)
{
	UiElement::SetElementPosition(position);
}

bool UiProgressBar::LoadFromXML(const XmlNode& node)
{
	if (!UiElement::LoadFromXML(node))
	{
		return false;
	}

	eFillOrientation fillOrientation = eFillOrientation::Vertical;
	const auto* orientationString = node.Child("FillOrientation");
	if (orientationString != nullptr)
	{
		if (orientationString->m_Content == "horizontal")
		{
			fillOrientation = eFillOrientation::Horizontal;
		}
	}

	auto* colourNode = node.Child("backgroundcolour");
	unsigned colour = 0x000000FF;
	if (colourNode != nullptr)
	{
		std::stringstream str;
		str << colourNode->m_Content;
		str >> std::hex >> colour;
	}

	m_background.setFillColor(static_cast<sf::Color>(colour));

	colourNode = node.Child("fillcolour");
	colour = ~0U;
	if (colourNode != nullptr)
	{
		std::stringstream str;
		str << colourNode->m_Content;
		str >> std::hex >> colour;
	}

	m_fill.setFillColor(static_cast<sf::Color>(colour));

	auto* sizeNode = node.Child("size");
	sf::Vector2f size = { 32.f, 93.f };
	if (sizeNode)
	{
		size.x = sizeNode->Attr("x", 0.f);
		size.y = sizeNode->Attr("y", 0.f);
	}

	m_background.setSize(size);
	m_fill.setSize(size - size / 4.f);


	m_background.setPosition(m_position);
	m_fill.setPosition(m_background.getPosition() + size / 4.f);

	AddDrawable(&m_background);
	AddDrawable(&m_fill);

	return true;
}
