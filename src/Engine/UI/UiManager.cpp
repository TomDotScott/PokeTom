#include "UiManager.h"
#include "../Globals.h"
#include <fstream>
#if BUILD_DEBUG
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#endif
#include <iostream>

#include "../Input/Mouse.h"
#include "UiButton.h"
#include "UiPanel.h"
#include "UiSprite.h"
#include "../Asserts.h"
#include "../CodeGen/Resources.hpp"
#include "../Parsers/XML/XmlDocument.h"

UiElement* UiManager::GetUiElement(const std::string& name) const
{
	if (!m_uiElements.contains(name))
	{
		return nullptr;
	}

	return m_uiElements.at(name);
}

UiButton* UiManager::GetUiButton(const std::string& name) const
{
	return dynamic_cast<UiButton*>(GetUiElement(name));
}

UiPanel* UiManager::GetUiPanel(const std::string& name) const
{
	return dynamic_cast<UiPanel*>(GetUiElement(name));
}

UiText* UiManager::GetUiText(const std::string& name) const
{
	return dynamic_cast<UiText*>(GetUiElement(name));
}

UiSprite* UiManager::GetUiSprite(const std::string& name) const
{
	return dynamic_cast<UiSprite*>(GetUiElement(name));
}


#if !BUILD_MASTER
void UiManager::DrawDebugText(sf::RenderWindow& window) const
{
	for (const auto& text : GetUiText("DEBUG_TEXT")->GetDrawablesList())
	{
		window.draw(*text);
	}
}
#endif

UiManager::UiManager()
{
	m_defaultUIInputs.Map(0, eInputType::Mouse, static_cast<int>(sf::Mouse::Button::Left));

	m_defaultUIInputs.OnButtonPressed(0, [this] { OnLeftClickPressed(); });
}

UiManager& UiManager::Get()
{
	static UiManager* uiManager = nullptr;
	if (uiManager == nullptr)
	{
		uiManager = new UiManager();
	}

	return *uiManager;
}

bool UiManager::Load(const std::filesystem::path& path)
{
	// HACKY, needs a proper system but oh well! Load all the fonts from Resources.cpp
	for (const auto& [resourceID, resourcePath] : fonts_resources)
	{
		if (!std::filesystem::exists(resourcePath))
		{
			std::cerr << "UiManager::Load: failed to load font " << resourceID << " as path " << resourcePath << " does not exist!\n";
			return false;
		}

		if (const auto id = std::string{ resourceID }; !m_fonts.contains(id))
		{
#if BUILD_DEBUG
			std::cout << "UiManager::LoadFont: Loading font \""<< id << "\", path=\"" << resourcePath<< "\"\n";
#endif

			m_fonts[id] = std::make_unique<sf::Font>(resourcePath);
		}
	}

	XmlDocument doc;
	doc.Load(path);
	return LoadFromXML(*doc.Root().Child("ui"));
}

bool UiManager::LoadElement(const XmlNode& node)
{
	UiElement* currentElement;

	// TODO: See if we can make this more strongly typed! It took me a while to find this block after 4 months, it would be nice to map something to an enum automatically for this code!
	if (node.m_Tag == "Panel")
	{
		currentElement = new UiPanel();
	}
	else if (node.m_Tag == "Sprite")
	{
		currentElement = new UiSprite();
	}
	else if (node.m_Tag == "Text")
	{
		currentElement = new UiText();
	}
	else if (node.m_Tag == "Button")
	{
		currentElement = new UiButton();
	}
	else
	{
		// We only care about our specific tags...
		return false;
	}

	if (!currentElement)
	{
		return false;
	}

	if (!currentElement->LoadFromXML(node))
	{
		// TODO: Proper cleanup
		delete currentElement;

		printf(" Error reading xml!");
		return false;
	}

	if (m_uiElements.contains(currentElement->GetName()))
	{
		printf(" UiManager: UI Element with name %s already exists!\n", currentElement->GetName().c_str());
		delete currentElement;
		return false;
	}

	m_uiElements[currentElement->GetName()] = currentElement;

	switch (currentElement->GetLayer())
	{
	case UiElement::eLayer::NONE:
		// TODO: Proper cleanup
		delete currentElement;

		printf(" UiManager: No layer set for UiElement %s\n", currentElement->GetName().c_str());
		return false;

	case UiElement::eLayer::BACKGROUND:
		m_backgroundElements.insert(currentElement->GetName());
		break;
	case UiElement::eLayer::MIDGROUND:
		m_midgroundElements.insert(currentElement->GetName());
		break;
	case UiElement::eLayer::FOREGROUND:
		m_foregroundElements.insert(currentElement->GetName());
		break;
	}

	return true;
}

void UiManager::Update()
{
	m_defaultUIInputs.Update();

	// TODO: Animation!
}


bool UiManager::LoadFromXML(const XmlNode& node)
{
	const auto children = node.Children();
	for (const auto& c : children)
	{
		if (!LoadElement(*c))
		{
			std::cerr << "UiManager::LoadFromXML - Failed to load element\n";
		}
	}

	return true;
}

void UiManager::RenderLayer(sf::RenderWindow& window, const std::set<std::string>& layerUIElementIDs) const
{
#if RENDER_SPRITES
	for (const auto& elementName : layerUIElementIDs)
	{
		UiElement* uiElement = m_uiElements.at(elementName);
		ASSERT(uiElement != nullptr);

		if (!uiElement->IsActive())
		{
			continue;
		}

#if BUILD_DEBUG
		if (uiElement->GetType() == UiElement::eType::Panel)
		{
			sf::RectangleShape debugRect(uiElement->GetSize());
			debugRect.setPosition(uiElement->GetPosition());
			debugRect.setFillColor(sf::Color::Magenta);
			window.draw(debugRect);
		}
#endif

		for (const auto* drawable : uiElement->GetDrawablesList())
		{
			window.draw(*drawable);
		}
	}
#endif
}

void UiManager::OnLeftClickPressed()
{
	for (const auto& [elementName, element] : m_uiElements)
	{
		if (element->GetType() != UiElement::eType::Button)
		{
			continue;
		}

		auto& button = dynamic_cast<UiButton&>(*element);

		const sf::Vector2f mousePosition = static_cast<sf::Vector2f>(Mouse::Get().GetPosition());
		const sf::Vector2f topLeft = button.GetPosition();
		const sf::Vector2f bottomRight = button.GetBottomRight();

		const bool inWidth = mousePosition.x >= topLeft.x && mousePosition.x <= bottomRight.x;
		const bool inHeight = mousePosition.y >= topLeft.y && mousePosition.y <= bottomRight.y;

		if (inWidth && inHeight)
		{
			button.OnLeftClickPressed();
		}
	}
}

void UiManager::RenderForeground(sf::RenderWindow& window) const
{
	RenderLayer(window, m_foregroundElements);
}

void UiManager::RenderMidground(sf::RenderWindow& window) const
{
	RenderLayer(window, m_midgroundElements);
}

void UiManager::RenderBackground(sf::RenderWindow& window) const
{
	RenderLayer(window, m_backgroundElements);
}

const sf::Font* UiManager::GetFont(const std::string& name) const
{
	if (!m_fonts.contains(name))
	{
		return nullptr;
	}
	return m_fonts.at(name).get();
}
