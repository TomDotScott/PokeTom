#include "UiElement.h"

#include <iostream>

#include "../Globals.h"

UiElement::UiElement(const eType type, UiElement* parent) :
	m_parent(parent),
	m_name("NO NAME SET"),
	m_layer(0),
	m_type(type)
{
	m_drawables.reserve(10);
}

int8_t UiElement::GetLayer() const
{
	return m_layer;
}

void UiElement::SetLayer(const int8_t layer)
{
	m_layer = layer;
}

UiElement::eType UiElement::GetType() const
{
	return m_type;
}

std::string UiElement::GetName() const
{
	return m_name;
}

void UiElement::SetElementPosition(const sf::Vector2f& position)
{
	m_absolutePosition = position;

	const sf::Vector2f offset{ m_parent->CalculateOffsetFromParents() };
	SetPosition(position + offset);
}

sf::Vector2f UiElement::GetAbsolutePosition() const
{
	return m_absolutePosition;
}

sf::Vector2f UiElement::CalculateOffsetFromParents() const
{
	sf::Vector2f offset{ 0.f, 0.f };

	const UiElement* current = m_parent;
	while (current != nullptr)
	{
		offset += current->GetAbsolutePosition();
		current = current->m_parent;
	}

	return offset;
}

const std::vector<const sf::Drawable*>& UiElement::GetDrawablesList() const
{
	return m_drawables;
}

void UiElement::AddDrawable(const sf::Drawable* drawable)
{
	m_drawables.emplace_back(drawable);
}

bool UiElement::LoadFromXML(const XmlNode& node)
{
	const auto* nameNode = node.Child("name");
	if (nameNode == nullptr)
	{
		std::cerr << "UiElement::LoadFromXML - No <name/> tag on the UiElement!\n";
		return false;
	}

	if (nameNode->m_Content.empty())
	{
		std::cerr << "UiElement::LoadFromXML - <name/> value was empty!\n";
		return false;
	}

	m_name = nameNode->m_Content;

	const auto* positionNode = node.Child("position");
	if (positionNode == nullptr)
	{
		std::cerr << "UiElement::LoadFromXML - No <position/> tag on the UiElement!\n";
		return false;
	}

	const auto* layerNode = node.Child("layer");
	if (layerNode != nullptr)
	{
		SetLayer(static_cast<int8_t>(std::stoi(layerNode->m_Content)));
	}
	else
	{
		std::cerr << "UiElement::LoadFromXML - Warning, no layer provided for UiElement " << GetName() << "! Setting to 0\n";
		SetLayer(0);
	}

	SetElementPosition(positionNode->Attr("x", "y", sf::Vector2f{}));
	return true;
}

UiElement* UiElement::GetParent() const
{
	return m_parent;
}

UiElement* UiElement::GetTopMostParent() const
{
	UiElement* parent = m_parent;

	if (parent == nullptr)
	{
		return nullptr;
	}

	while (parent->GetParent() != nullptr)
	{
		parent = parent->GetParent();
	}

	return parent;
}

void UiElement::SetParent(UiElement* parent)
{
	m_parent = parent;
}
