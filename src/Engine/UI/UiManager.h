#ifndef UI_MANAGER_H
#define UI_MANAGER_H
#include <filesystem>
#include <map>
#include <set>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "UiElement.h"
#include "../Input/InputMapper.h"

class UiManager final : public ISerialisable
{
public:
	static UiManager& Get();

	static constexpr int8_t k_topLayer = 0x7F;
	static constexpr int8_t k_bottomLayer = ~0x7F;

	bool Load(const std::filesystem::path& path);
	void Update();

	// TODO: There has to be a better way to do this? Maybe a renderer class with a vector of sf::drawables in order?
	void RenderAll(sf::RenderWindow& window) const;
	void RenderLayer(sf::RenderWindow& window, int8_t layer) const;

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
	std::map<int8_t, std::set<std::string>> m_sortOrderElements;

	InputMapper m_defaultUIInputs;

	bool LoadFromXML(const XmlNode& node) override;
	bool LoadElement(const XmlNode& node);

	// TODO: Do we need to have a remove as well depending on state? There might be a LOT of UI in a game...
	void InsertElementIntoLayer(const UiElement* current);

	void RenderElement(sf::RenderWindow& window, const UiElement* uiElement, int8_t layer) const;

	void OnLeftClickPressed();
};

#define UIMANAGER UiManager::Get()

#endif
