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
#include "../XML/XmlDocument.h"

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
		delete currentElement;

		printf(" UiManager: UI Element with name %s already exists!\n", currentElement->GetName().c_str());
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

bool UiManager::LoadFont(const XmlNode& node)
{
	const auto* nameNode = node.Child("name");
	if (nameNode == nullptr)
	{
		return false;
	}

	const std::string name = nameNode->m_Content;

	const auto* pathNode = node.Child("path");
	if (pathNode == nullptr)
	{
		return false;
	}
	const std::filesystem::path path = pathNode->m_Content;

	if (!m_fonts.contains(name))
	{
#if BUILD_DEBUG
		printf(" UiManager::LoadFont: Loading font \"%s\", path=\"%ls\"\n", name.c_str(), path.c_str());
#endif

		m_fonts[name] = std::make_unique<sf::Font>(path);
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
		if (c->m_Tag == "Font")
		{
			if (!LoadFont(*c))
			{
				std::cerr << "UiManager::LoadFromXML - Failed to load font " << c->m_Content << "\n";
				return false;
			}
		}
		else
		{
			if (!LoadElement(*c))
			{
				std::cerr << "UiManager::LoadFromXML - Failed to load element\n";
			}
		}
	}

	return true;
}

void UiManager::RenderLayer(sf::RenderWindow& window, const std::set<std::string>& layerUIElementIDs) const
{
#if RENDER_SPRITES
	for (const auto& elementName : layerUIElementIDs)
	{
#if BUILD_DEBUG
		if (m_uiElements.at(elementName)->GetType() == UiElement::eType::Panel)
		{
			sf::RectangleShape debugRect(m_uiElements.at(elementName)->GetSize());
			debugRect.setPosition(m_uiElements.at(elementName)->GetPosition());
			debugRect.setFillColor(sf::Color::Magenta);
			window.draw(debugRect);
		}
#endif

		for (const auto* drawable : m_uiElements.at(elementName)->GetDrawablesList())
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
