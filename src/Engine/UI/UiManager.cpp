#include "UiManager.h"
#include "../Globals.h"
#include <fstream>
#if BUILD_DEBUG
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#endif
#include "../Input/Mouse.h"
#include "UiButton.h"
#include "UiPanel.h"
#include "UiSprite.h"

UiElement* UiManager::GetUiElement(const std::string& name) const
{
	if (m_uiElements.find(name) == m_uiElements.end())
	{
		return nullptr;
	}

	return m_uiElements.at(name);
}

UiButton* UiManager::GetUiButton(const std::string& name) const
{
	return static_cast<UiButton*>(GetUiElement(name));
}

UiPanel* UiManager::GetUiPanel(const std::string& name) const
{
	return static_cast<UiPanel*>(GetUiElement(name));
}

UiText* UiManager::GetUiText(const std::string& name) const
{
	return static_cast<UiText*>(GetUiElement(name));
}

UiSprite* UiManager::GetUiSprite(const std::string& name) const
{
	return static_cast<UiSprite*>(GetUiElement(name));
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

UiManager::LastXmlDetails UiManager::GetLastXmlDetails() const
{
	return m_lastXmlDetails;
}

UiManager::UiManager() :
	m_lastXmlDetails()
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
	std::string line, text;
	std::ifstream in(path);
	while (std::getline(in, line))
	{
		text += line + "\n";
	}

	if (text.empty())
	{
		return false;
	}

	const char* content = text.c_str();

	const size_t content_length = strlen(content);

	hoxml_context_t* hoxml_context = new hoxml_context_t();
	const auto buffer = static_cast<char*>(malloc(content_length * 2));

	hoxml_init(hoxml_context, buffer, content_length * 2);

	m_lastXmlDetails = { content, content_length };

	// Loop until the "end of document" code is returned
	hoxml_code_t code = hoxml_parse(hoxml_context, content, content_length);
	while (code != HOXML_END_OF_DOCUMENT)
	{
		if (code == HOXML_ELEMENT_BEGIN)
		{
#if BUILD_DEBUG
			printf(" Opened <%s>\n", hoxml_context->tag);
#endif

			if (strcmp("Font", hoxml_context->tag) == 0)
			{
				LoadFont(hoxml_context, content, content_length);
			}
			else
			{
				LoadElement(hoxml_context, content, content_length);
			}
		}
		if (code == HOXML_ERROR_TAG_MISMATCH)
		{
			printf(" UiManager: Start tag did not match end tag on line %u\n", hoxml_context->line);
			return false;
		}

		code = hoxml_parse(hoxml_context, content, content_length);
	}

	free(buffer);
	return true;
}

bool UiManager::LoadElement(hoxml_context_t*& context, const char* xml, const size_t xmlLength)
{
	UiElement* currentElement = nullptr;

	// TODO: See if we can make this more strongly typed! It took me a while to find this block after 4 months, it would be nice to map something to an enum automatically for this code!
	if (strcmp("Panel", context->tag) == 0)
	{
		currentElement = new UiPanel();
	}
	else if (strcmp("Sprite", context->tag) == 0)
	{
		currentElement = new UiSprite();
	}
	else if (strcmp("Text", context->tag) == 0)
	{
		currentElement = new UiText();
	}
	else if (strcmp("Button", context->tag) == 0)
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

	if (!currentElement->Load(context, xml, xmlLength))
	{
		// TODO: Proper cleanup
		delete currentElement;

		printf(" Error reading xml!");
		return false;
	}

	if (m_uiElements.find(currentElement->GetName()) != m_uiElements.end())
	{
		delete currentElement;

		printf(" UiManager: UI Element with name %s already exists!\n", currentElement->GetName().c_str());
		return false;
	}

	m_uiElements[currentElement->GetName()] = currentElement;

	switch (currentElement->GetLayer()) {
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

bool UiManager::LoadFont(hoxml_context_t*& context, const char* xml, const size_t xmlLength)
{
	hoxml_code_t code = HOXML_ELEMENT_BEGIN;

	std::string name;
	std::filesystem::path path;

	do
	{
		if (code == HOXML_ELEMENT_END)
		{
			if (strcmp("name", context->tag) == 0)
			{
				name = context->content;
			}
			else if (strcmp("path", context->tag) == 0)
			{
				path = context->content;
			}

			if (strcmp("Font", context->tag) != 0)
			{
				code = hoxml_parse(context, xml, xmlLength);
				continue;
			}

			if (name.empty())
			{
				printf(" UiManager::LoadFont: Trying to load a font with no name tag on line %u.\n", context->line);
				return false;
			}
			if (path.empty())
			{
				printf(" UiManager::LoadFont: Trying to load font %s with no path tag on line %u.\n", name.c_str(), context->line);
				return false;
			}

			if (m_fonts.find(name) == m_fonts.end())
			{
#if BUILD_DEBUG
				printf(" UiManager::LoadFont: Loading font \"%s\", path=\"%ls\"\n", name.c_str(), path.c_str());
#endif
				sf::Font* font = new sf::Font(path);

				m_fonts[name] = font;
			}

			// Reached the end!
			return true;
		}

		code = hoxml_parse(context, xml, xmlLength);
	} while (code != HOXML_END_OF_DOCUMENT);

	return false;
}

void UiManager::Update()
{
	m_defaultUIInputs.Update();

	// TODO: Animation!
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

		auto& button = static_cast<UiButton&>(*element);

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
	if (m_fonts.find(name) == m_fonts.end())
	{
		return nullptr;
	}
	return m_fonts.at(name);
}


