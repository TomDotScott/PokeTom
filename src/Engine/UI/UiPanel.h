#ifndef UI_PANEL_H
#define UI_PANEL_H

#include "UiElement.h"
#include "UiSprite.h"
#include "UiText.h"

class UiPanel : public UiElement
{
public:
	UiPanel();

	void SetPosition(const sf::Vector2f& position) override;

	sf::Vector2f GetSize() const override;
	void SetSize(const sf::Vector2f& size);

	UiText* GetUiText(const std::string& name);

private:
	UiSprite* m_sprite;

	std::vector<OffsetUiText> m_text;

	sf::Vector2f m_size;

	bool ParseAttribute(hoxml_context_t*& context) override;
	bool ParseBeginElement(hoxml_context_t*& context) override;
	bool ParseEndElement(hoxml_context_t*& context) override;
};

#endif