#ifndef UI_MANAGER_H
#define UI_MANAGER_H
#include <filesystem>
#include <set>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "UiElement.h"

#include "UiButton.h"
#include "UiPanel.h"
#include "UiSprite.h"
#include "UiText.h"

#include "../Input/InputMapper.h"

class UiManager : public ISerialisable
{
public:
	static UiManager& Get();

	bool Load(const std::filesystem::path& path);
	void Update();

	// TODO: There has to be a better way to do this? Maybe a renderer class with a vector of sf::drawables in order?
	void RenderForeground(sf::RenderWindow& window) const;
	void RenderMidground(sf::RenderWindow& window) const;
	void RenderBackground(sf::RenderWindow& window) const;

	const sf::Font* GetFont(const std::string& name) const;

	UiElement* GetElement(const std::string& name) const
	{
		if (!m_uiElements.contains(name))
		{
			return nullptr;
		}

		return m_uiElements.at(name);
	}

	template<typename T>
	T* GetElement(const std::string& name) const
		requires (std::is_base_of_v<UiElement, T>)
	{
		return dynamic_cast<T*>(GetElement(name));
	}

#if !BUILD_MASTER
	void DrawDebugText(sf::RenderWindow& window) const;
#endif

private:
	UiManager();

	std::unordered_map<std::string, std::unique_ptr<sf::Font>> m_fonts;

	std::unordered_map<std::string, UiElement*> m_uiElements;
	std::set<std::string> m_backgroundElements;
	std::set<std::string> m_midgroundElements;
	std::set<std::string> m_foregroundElements;

	InputMapper m_defaultUIInputs;

	bool LoadFromXML(const XmlNode& node) override;
	bool LoadElement(const XmlNode& node);

	void InsertElementIntoLayer(const std::string& elementName, UiElement::eLayer layer);

	void RenderLayer(sf::RenderWindow& window, UiElement::eLayer layer) const;
	void RenderElement(sf::RenderWindow& window, const UiElement* uiElement, UiElement::eLayer layer) const;

	void OnLeftClickPressed();
};

#define UIMANAGER UiManager::Get()

#endif
