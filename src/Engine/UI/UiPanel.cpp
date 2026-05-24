#include "UiPanel.h"

#include <iostream>

#include "UiManager.h"
#include "../Asserts.h"

UiPanel::UiPanel(UiElement* parent) :
	UiElement(eType::Panel, parent)
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

	if (!LoadChildrenOfType<UiSprite>(node, "Sprite")) { return false; }
	if (!LoadChildrenOfType<UiText>(node, "Text")) { return false; }
	if (!LoadChildrenOfType<UiButton>(node, "Button")) { return false; }
	if (!LoadChildrenOfType<UiPanel>(node, "Panel")) { return false; }

	SetElementPosition(m_position);
	return true;
}

void UiPanel::OnActivate()
{
	UiElement::OnActivate();

	// Activate all the children
	for (const auto& child : m_children)
	{
		child->OnActivate();
	}
}
