#ifndef UI_SPRITE_H
#define UI_SPRITE_H
#include <SFML/Graphics/Sprite.hpp>

#include "UiElement.h"

class UiSprite : public UiElement
{
public:
	UiSprite();

	void SetPosition(const sf::Vector2f& position) override;

	void SetScale(const sf::Vector2f& scale) const;

	sf::Vector2f GetSize() const override;

private:
	sf::Sprite* m_sprite;

	// The scale of the texture vs the screen
	sf::Vector2f m_screenScaleFactor;

	// The scale of the sprite from the xml file
	sf::Vector2f m_scaleFactorFromXml;

	bool ParseEndElement(hoxml_context_t*& context) override;
	bool ParseAttribute(hoxml_context_t*& context) override;
};

#endif