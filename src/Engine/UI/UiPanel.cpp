#include "UiPanel.h"

#include <iostream>

#include "UiManager.h"
#include "../Asserts.h"

UiPanel::UiPanel() :
	UiElement(eType::Panel)
{
	SetLayer(eLayer::MIDGROUND);
}

void UiPanel::SetElementPosition(const sf::Vector2f& position)
{
	m_absolutePosition = position;

	UiElement::SetPosition(position + CalculateOffsetFromParents());

	for (const std::unique_ptr<UiElement>& child : m_children)
	{
		child->RecalculatePositionAfterParentMoved();
	}
}

void UiPanel::RecalculatePositionAfterParentMoved()
{
	SetElementPosition(m_absolutePosition);
}

sf::Vector2f UiPanel::GetSize() const
{
	return m_size;
}

void UiPanel::SetSize(const sf::Vector2f& size)
{
	m_size = size;
}

UiElement* UiPanel::GetChild(const std::string& name) const
{
	for (auto& child : m_children)
	{
		if (child->GetName() == name)
		{
			// TODO: Returning a pointer to an element in a vector is not always the best idea...
			return child.get();
		}
	}
	return nullptr;
}

const std::vector<std::unique_ptr<UiElement>>& UiPanel::GetChildren() const
{
	return m_children;
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

	// TODO: Make this support any arbitrary children
	const auto spriteNodes = node.Children("Sprite");
	for (const auto& c : spriteNodes)
	{
		if (c == nullptr)
		{
			continue;
		}

		// This is a kinda hideous line of code
		UiSprite* newSprite = dynamic_cast<UiSprite*>(m_children.emplace_back(std::make_unique<UiSprite>(this)).get());
		if (!newSprite->LoadFromXML(*c))
		{
#if BUILD_DEBUG
			ASSERT_MSG(false, "UiPanel: Error loading sprite in panel %s", GetName().c_str());
#endif
			return false;
		}

		for (const sf::Drawable* drawable : newSprite->GetDrawablesList())
		{
			AddDrawable(drawable);
		}
	}

	const auto textChildren = node.Children("Text");
	for (const auto& c : textChildren)
	{
		if (c == nullptr)
		{
			continue;
		}

		// This is a kinda hideous line of code
		UiText* newText = dynamic_cast<UiText*>(m_children.emplace_back(std::make_unique<UiText>(this)).get());

		if (!newText->LoadFromXML(*c))
		{
			printf(" UiPanel: Error loading Text");
			return false;
		}

		for (const sf::Drawable* drawable : newText->GetDrawablesList())
		{
			AddDrawable(drawable);
		}
	}

	SetElementPosition(m_position);
	return true;
}
