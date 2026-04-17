#include "UiPanel.h"

#include <iostream>

#include "UiManager.h"
#include "../Globals.h"

UiPanel::UiPanel() :
	UiElement(eType::Panel),
	m_sprite(nullptr)
{
	SetLayer(eLayer::MIDGROUND);
}

void UiPanel::SetElementPosition(const sf::Vector2f& position)
{
	UiElement::SetPosition(position);

	if (m_sprite)
	{
		m_sprite->SetElementPosition(m_position);
	}

	for (const auto& [text, offset] : m_text)
	{
		sf::Vector2f anchor = m_position;

		if (text->GetAlignment() == UiText::eAlignment::Centre)
		{
			anchor = m_position + m_size / 2.f;
		}

		text->SetElementPosition(anchor + offset);
	}
}

sf::Vector2f UiPanel::GetSize() const
{
	return m_size;
}

void UiPanel::SetSize(const sf::Vector2f& size)
{
	m_size = size;
}

UiText* UiPanel::GetUiText(const std::string& name)
{
	for (auto& [uiText, offsetPosition] : m_text)
	{
		if (uiText->GetName() == name)
		{
			return uiText.get();
		}
	}
	return nullptr;
}

bool UiPanel::LoadFromXML(const XmlNode& node)
{
	if (!UiElement::LoadFromXML(node))
	{
		return false;
	}

	const auto* sizeNode = node.Child("size");
	if (sizeNode == nullptr)
	{
		std::cerr << "UiPanel::LoadFromXML - No size provided for element " << GetName() << "\n";
		return false;
	}

	m_size = sizeNode->Attr("x", "y", sf::Vector2f{ 69, 69 });

	const auto textChildren = node.Children("Text");
	m_text.reserve(textChildren.size());

	for (const auto& c : textChildren)
	{
		OffsetUiText& newText = m_text.emplace_back();

		newText.m_text = std::make_unique<UiText>(this);

		if (!newText.m_text->LoadFromXML(*c))
		{
			printf(" UiPanel: Error loading Text");
			return false;
		}

		newText.m_offset = newText.m_text->GetPosition();
	}

	const auto* spriteNode = node.Child("Sprite");
	if (spriteNode != nullptr)
	{
		m_sprite = std::make_unique<UiSprite>();
		if (!m_sprite->LoadFromXML(*spriteNode))
		{
			std::cerr << "UiPanel: Error loading sprite in panel " << GetName() << "\n";
			return false;
		}
	}

	SetElementPosition(m_position);

	if (m_sprite)
	{
		for (const sf::Drawable* drawable : m_sprite->GetDrawablesList())
		{
			AddDrawable(drawable);
		}
	}

	for (const auto& [text, offset] : m_text)
	{
		for (const sf::Drawable* drawable : text->GetDrawablesList())
		{
			AddDrawable(drawable);
		}
	}

	return true;
}
