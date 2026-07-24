#ifndef UIPROGRESSBAR_H
#define UIPROGRESSBAR_H

#include <SFML/Graphics/RectangleShape.hpp>

#include "UiElement.h"

class UiProgressBar : public UiElement
{
public:
	enum class eFillOrientation
	{
		Horizontal,
		Vertical
	};

	UiProgressBar(UiElement* parent = nullptr);
	sf::Vector2f GetSize() const override;
	void RecalculatePositionAfterParentMoved() override;

	void OnActivate() override;
	void OnDeactivate() override;

	void SetElementPosition(const sf::Vector2f& position) override;
	bool LoadFromXML(const XmlNode& node) override;

private:
	sf::RectangleShape m_fill;
	sf::RectangleShape m_background;

};


#endif // UIPROGRESSBAR_H
