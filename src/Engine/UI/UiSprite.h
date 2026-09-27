#ifndef UI_SPRITE_H
#define UI_SPRITE_H
#include <SFML/Graphics/Sprite.hpp>

#include "UiElement.h"
#include "../Hash.h"

class UiSprite : public UiElement
{
public:
	UiSprite(UiElement* parent = nullptr);

	void SetElementPosition(const sf::Vector2f& position) override;
	void RecalculatePositionAfterParentMoved() override;

	void SetScale(const sf::Vector2f& scale) const;

	sf::Vector2f GetSize() const override;

	bool LoadFromXML(const XmlNode& node) override;

	void SetTexture(hash_type resourceID, sf::IntRect textureBounds);

private:
	sf::Sprite* m_sprite;
	sf::Vector2f m_scaleFactorFromXml;

	bool LoadTexture(const std::string& resourceID);
};

#endif
