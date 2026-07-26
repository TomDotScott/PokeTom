#include "UiProgressBar.h"

#include "../Asserts.h"
#include "../Maths.h"


UiProgressBar::UiProgressBar(UiElement* parent) :
	UiElement(eType::ProgressBar, parent),
	m_maxFillSize(0.f),
	m_fillOrientation(eFillOrientation::Vertical),
	m_progress(0.f)
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

	const auto* orientationString = node.Child("FillOrientation");
	if (orientationString != nullptr)
	{
		if (orientationString->m_Content == "horizontal")
		{
			m_fillOrientation = eFillOrientation::Horizontal;
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
		size = sizeNode->Attr("x", "y", sf::Vector2f{});
	}

	auto* marginNode = node.Child("margin");
	sf::Vector2f margin = {};
	if (marginNode)
	{
		margin = marginNode->Attr("x", "y", margin);
	}

	m_background.setSize(size);

	// Calculate the size based off the margin
	const float xSize = size.x - margin.x * 2.f;
	const float ySize = size.y - margin.y * 2.f;
	const sf::Vector2f offsetVector{ xSize, ySize };

	m_fill.setSize(offsetVector);

	m_maxFillSize = m_fillOrientation == eFillOrientation::Horizontal ? m_fill.getSize().x : m_fill.getSize().y;

	m_background.setPosition(m_position);
	m_fill.setPosition(m_background.getPosition() + margin);

	AddDrawable(&m_background);
	AddDrawable(&m_fill);

	return true;
}

void UiProgressBar::SetProgress(const float progress)
{
	m_progress = progress;
	UpdateFill();
}

void UiProgressBar::SetProgress(const float val, const float maxVal, const bool invert)
{
	ASSERT(maxVal > 0.f);
	ASSERT(val >= 0.f);

	float progress = maths::Clamp(val / maxVal, 0.f, 1.0f);

	if (invert)
	{
		progress = 1.0f - progress;
	}

	SetProgress(progress);
}

float UiProgressBar::GetProgress(const bool invert) const
{
	if (invert)
	{
		return 1 - m_progress;
	}

	return m_progress;
}

void UiProgressBar::UpdateFill()
{
	const float newSize = m_progress * m_maxFillSize;

	if (m_fillOrientation == eFillOrientation::Vertical)
	{
		m_fill.setSize({ m_fill.getSize().x, newSize });
	}
	else
	{
		m_fill.setSize({ newSize, m_fill.getSize().y });
	}
}
