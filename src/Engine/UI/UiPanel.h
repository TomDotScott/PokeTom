#ifndef UI_PANEL_H
#define UI_PANEL_H

#include "UiElement.h"
#include "UiSprite.h"
#include "UiText.h"

class UiPanel : public UiElement
{
public:
	UiPanel();

	void SetElementPosition(const sf::Vector2f& position) override;

	sf::Vector2f GetSize() const override;
	void SetSize(const sf::Vector2f& size);

	UiText* GetUiText(const std::string& name);

private:
	std::unique_ptr<UiSprite> m_sprite;

	std::vector<OffsetUiText> m_text;

	sf::Vector2f m_size;

	bool LoadFromXML(const XmlNode& node) override;
};

#endif