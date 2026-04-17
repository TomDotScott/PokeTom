#include "UiElement.h"

#include <iostream>

#include "../Globals.h"

UiElement::UiElement(const eType type) :
	m_layer(eLayer::NONE),
	m_type(type)
{
	m_drawables.reserve(10);
}

UiElement::eLayer UiElement::GetLayer() const
{
	return m_layer;
}

void UiElement::SetLayer(const eLayer layer)
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
	SetPosition(position);
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

	m_name = nameNode->m_Content;
	if (m_name.empty())
	{
		std::cerr << "UiElement::LoadFromXML - <name/> value was empty!\n";
		return false;
	}

	const auto* positionNode = node.Child("position");
	if (positionNode == nullptr)
	{
		std::cerr << "UiElement::LoadFromXML - No <position/> tag on the UiElement!\n";
		return false;
	}

	SetElementPosition(positionNode->Attr("x", "y", sf::Vector2f{}));
	return true;
}
