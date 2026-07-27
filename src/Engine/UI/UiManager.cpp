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
#include "UiProgressBar.h"
#include "UiSprite.h"
#include "../Asserts.h"
#include "../EngineEvents.h"
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

UiManager::UiManager() :
	m_uiView(sf::Vector2f{ 0.f, 0.f },
	         static_cast<sf::Vector2f>(REFERENCE_SCREEN_SIZE)
	)
{
	m_defaultUIInputs.Map(0, eInputType::Mouse, static_cast<int>(sf::Mouse::Button::Left));

	m_defaultUIInputs.OnButtonPressed(0, [this] { OnLeftClickPressed(); });

	engine_events::OnWindowResized.On([this](const sf::Vector2u newSize) { OnScreenResized(newSize); });
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
	else if (node.m_Tag == "ProgressBar")
	{
		currentElement = new UiProgressBar();
	}
	else
	{
		ASSERT_MSG(false, "Unknown button type %s\n", node.m_Tag.c_str());

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

	ASSERT(!m_uiElements.contains(currentElement->GetName()));
	if (m_uiElements.contains(currentElement->GetName()))
	{
		printf(" UiManager: UI Element with name %s already exists!\n", currentElement->GetName().c_str());
		delete currentElement;
		return false;
	}

	m_uiElements[currentElement->GetName()] = currentElement;

	InsertElementIntoLayer(currentElement);

	return true;
}

void UiManager::InsertElementIntoLayer(const UiElement* current)
{
	const std::string parentName = (current->GetTopMostParent() != nullptr ? current->GetTopMostParent() : current)->
		GetName();

	m_sortOrderElements[current->GetLayer()].insert(parentName);

	// If it is a Panel, make sure the children are sorted properly as well!
	if (const UiPanel* panel = dynamic_cast<const UiPanel*>(current); panel != nullptr)
	{
		for (const std::unique_ptr<UiElement>& child : panel->GetChildren())
		{
			InsertElementIntoLayer(child.get());
		}
	}
}

void UiManager::Update(const float deltaTime, const sf::RenderWindow& window)
{
	m_lastMousePositionInUiSpace = window.mapPixelToCoords(Mouse::Get().GetPosition(), m_uiView);

	m_defaultUIInputs.Update();
	// TODO: Animation!
}

void UiManager::OnScreenResized(sf::Vector2u /*newSize*/)
{
	m_uiView.setCenter(static_cast<sf::Vector2f>(REFERENCE_SCREEN_SIZE) / 2.f);
	m_uiView.setViewport(GRAPHIC_SETTINGS.GetLetterboxViewport());
}

void UiManager::RenderAll(sf::RenderWindow& window) const
{
	for (int8_t i = k_bottomLayer; i < k_topLayer; ++i)
	{
		RenderLayer(window, i);
	}

	window.setView({
		static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre),
		static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize)
	});
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

void UiManager::RenderLayer(sf::RenderWindow& window, const int8_t layer) const
{
	if (!m_sortOrderElements.contains(layer))
	{
		return;
	}

	window.setView(m_uiView);

#if UIMANAGER_DEBUG_SPEW
	printf("Moving onto layer %d\n", layer);
#endif

#if RENDER_SPRITES
	const std::set<std::string>& uiElementNames = m_sortOrderElements.at(layer);

	for (const auto& elementName : uiElementNames)
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

void UiManager::RenderElement(sf::RenderWindow& window, const UiElement* uiElement, const int8_t layer) const
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
		printf("Drawing %s\n", uiElement->GetName().c_str());
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

		const sf::Vector2f topLeft = button.GetPosition();
		const sf::Vector2f bottomRight = button.GetBottomRight();

		const bool inWidth = m_lastMousePositionInUiSpace.x >= topLeft.x && m_lastMousePositionInUiSpace.x <=
			bottomRight.x;
		const bool inHeight = m_lastMousePositionInUiSpace.y >= topLeft.y && m_lastMousePositionInUiSpace.y <=
			bottomRight.y;

		if (inWidth && inHeight)
		{
			button.OnLeftClickPressed();
		}
	}
}

const sf::Font* UiManager::GetFont(const std::string& name) const
{
	if (!m_fonts.contains(name))
	{
		return nullptr;
	}
	return m_fonts.at(name).get();
}
