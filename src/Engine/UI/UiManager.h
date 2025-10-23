#ifndef UI_MANAGER_H
#define UI_MANAGER_H
#include <filesystem>
#include <set>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "UiButton.h"
#include "UiPanel.h"
#include "UiText.h"
#include "../../Libs/hoxml.h"
#include "../Input/InputMapper.h"

class UiManager
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

	UiElement* GetUiElement(const std::string& name) const;
	UiButton* GetUiButton(const std::string& name) const;
	UiPanel* GetUiPanel(const std::string& name) const;
	UiText* GetUiText(const std::string& name) const;
	UiSprite* GetUiSprite(const std::string& name) const;

#if !BUILD_MASTER
	void DrawDebugText(sf::RenderWindow& window) const;
#endif

	struct LastXmlDetails
	{
		// This pointer CAN be null
		const char* text;
		size_t length;
	};

	LastXmlDetails GetLastXmlDetails() const;

private:
	UiManager();

	std::unordered_map<std::string, sf::Font*> m_fonts;

	std::unordered_map<std::string, UiElement*> m_uiElements;
	std::set<std::string> m_backgroundElements;
	std::set<std::string> m_midgroundElements;
	std::set<std::string> m_foregroundElements;

	LastXmlDetails m_lastXmlDetails;

	InputMapper m_defaultUIInputs;

	bool LoadElement(hoxml_context_t*& context, const char* xml, size_t xmlLength);
	bool LoadFont(hoxml_context_t*& context, const char* xml, size_t xmlLength);
	void RenderLayer(sf::RenderWindow& window, const std::set<std::string>& layerUIElementIDs) const;

	void OnLeftClickPressed();
};

#define UIMANAGER UiManager::Get()

#endif
