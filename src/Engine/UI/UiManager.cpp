#include "UiManager.h"
#include "../Globals.h"
#include <fstream>
#if BUILD_DEBUG
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#endif
#include <iostream>
#include <ranges>

#include "../Input/Mouse.h"
#include "UiButton.h"
#include "UiPanel.h"
#include "UiSprite.h"
#include "../Asserts.h"
#include "../CodeGen/Resources.hpp"
#include "../Parsers/XML/XmlDocument.h"

#define UIMANAGER_DEBUG_SPEW 0
#define RENDER_PANEL_DEBUG 0

#if !BUILD_MASTER
void UiManager::DrawDebugText(sf::RenderWindow& window) const
{
	for (const auto& text : GetElement("DEBUG_TEXT")->GetDrawablesList())
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
			std::cerr << "UiManager::Load: failed to load font " << resourceID << " as path " << resourcePath <<
				" does not exist!\n";
			return false;
		}

		if (const auto id = std::string{ resourceID }; !m_fonts.contains(id))
		{
#if UIMANAGER_DEBUG_SPEW
			std::cout << "UiManager::LoadFont: Loading font \"" << id << "\", path=\"" << resourcePath << "\"\n";
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

	ASSERT_MSG(currentElement->GetLayer() != UiElement::eLayer::NONE, "UiManager: No layer set for UiElement %s",
	           currentElement->GetName().c_str());
	if (currentElement->GetLayer() == UiElement::eLayer::NONE)
	{
		delete currentElement;
		return false;
	}


	m_uiElements[currentElement->GetName()] = currentElement;

	// If it is a Panel, make sure the children are sorted properly
	if (const UiPanel* panel = dynamic_cast<const UiPanel*>(currentElement); panel != nullptr)
	{
		for (const std::unique_ptr<UiElement>& child : panel->GetChildren())
		{
			InsertElementIntoLayer(currentElement->GetName(), child->GetLayer());
		}
	}
	else
	{
		InsertElementIntoLayer(currentElement->GetName(), currentElement->GetLayer());
	}

	return true;
}

void UiManager::InsertElementIntoLayer(const std::string& elementName, const UiElement::eLayer layer)
{
	ASSERT(layer != UiElement::eLayer::NONE);

	switch (layer)
	{
	case UiElement::eLayer::BACKGROUND:
		m_backgroundElements.insert(elementName);
		break;
	case UiElement::eLayer::MIDGROUND:
		m_midgroundElements.insert(elementName);
		break;
	case UiElement::eLayer::FOREGROUND:
		m_foregroundElements.insert(elementName);
		break;
	}
}

void UiManager::Update()
{
	m_defaultUIInputs.Update();

	// TODO: Animation!
}


bool UiManager::LoadFromXML(const XmlNode& node)
{
	for (const auto& c : node.Children())
	{
		if (!LoadElement(*c))
		{
			std::cerr << "UiManager::LoadFromXML - Failed to load element\n";
		}
	}

	return true;
}

void UiManager::RenderLayer(sf::RenderWindow& window, const UiElement::eLayer layer) const
{
#if RENDER_SPRITES
	ASSERT(layer != UiElement::eLayer::NONE);

	const std::set<std::string>* uiElementNames = nullptr;
	switch (layer)
	{
	case UiElement::eLayer::BACKGROUND:
		uiElementNames = &m_backgroundElements;
		break;
	case UiElement::eLayer::MIDGROUND:
		uiElementNames = &m_midgroundElements;
		break;
	case UiElement::eLayer::FOREGROUND:
		uiElementNames = &m_foregroundElements;
		break;
	default: ;
	}

	for (const auto& elementName : *uiElementNames)
	{
		UiElement* uiElement = m_uiElements.at(elementName);
		ASSERT(uiElement != nullptr);

#if RENDER_PANEL_DEBUG
		if (uiElement->GetType() == UiElement::eType::Panel && uiElement->IsActive())
		{
			sf::RectangleShape debugRect(uiElement->GetSize());
			debugRect.setPosition(uiElement->GetPosition());
			debugRect.setFillColor({
				sf::Color::Magenta.r,
				sf::Color::Magenta.g,
				sf::Color::Magenta.b,
				64
			});
			window.draw(debugRect);
		}
#endif

		RenderElement(window, uiElement, layer);
	}
#endif
}

void UiManager::RenderElement(sf::RenderWindow& window, const UiElement* uiElement, const UiElement::eLayer layer) const
{
	if (uiElement->GetType() != UiElement::eType::Panel && uiElement->GetLayer() != layer)
	{
		return;
	}

	if (!uiElement->IsActive())
	{
		return;
	}

	if (const UiPanel* panel = dynamic_cast<const UiPanel*>(uiElement); panel != nullptr)
	{
		for (const std::unique_ptr<UiElement>& child : panel->GetChildren())
		{
#if UIMANAGER_DEBUG_SPEW
			printf("===> Children found! Attempting to render %s\n", child->GetName().c_str());
#endif

			RenderElement(window, child.get(), layer);
		}
	}
	else
	{
#if UIMANAGER_DEBUG_SPEW
		switch (layer)
		{
		case UiElement::eLayer::BACKGROUND:
			printf("Drawing %s in the BACKGROUND\n", uiElement->GetName().c_str());
			break;
		case UiElement::eLayer::MIDGROUND:
			printf("Drawing %s in the MIDGROUND\n", uiElement->GetName().c_str());
			break;
		case UiElement::eLayer::FOREGROUND:
			printf("Drawing %s in the FOREGROUND\n", uiElement->GetName().c_str());
			break;
		}
#endif

		for (const auto* drawable : uiElement->GetDrawablesList())
		{
			window.draw(*drawable);
		}
	}
}

void UiManager::OnLeftClickPressed()
{
	for (const auto& element : m_uiElements | std::views::values)
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
	RenderLayer(window, UiElement::eLayer::FOREGROUND);
}

void UiManager::RenderMidground(sf::RenderWindow& window) const
{
	RenderLayer(window, UiElement::eLayer::MIDGROUND);
}

void UiManager::RenderBackground(sf::RenderWindow& window) const
{
	RenderLayer(window, UiElement::eLayer::BACKGROUND);
}

const sf::Font* UiManager::GetFont(const std::string& name) const
{
	if (!m_fonts.contains(name))
	{
		return nullptr;
	}
	return m_fonts.at(name).get();
}
