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
	void RecalculatePositionAfterParentMoved() override;

	sf::Vector2f GetSize() const override;
	void SetSize(const sf::Vector2f& size);

	UiElement* GetChild(const std::string& name) const;

private:
	std::vector<std::unique_ptr<UiElement>> m_children;

	sf::Vector2f m_size;

	bool LoadFromXML(const XmlNode& node) override;
};

#endif