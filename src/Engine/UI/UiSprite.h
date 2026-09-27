#ifndef UI_SPRITE_H
#define UI_SPRITE_H
#include <map>
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


	// TODO: What implications does setting it to a MUCH larger texture have?
	void SetTexture(size_t textureIndex);
	void SetTexture(const std::string& textureID);

private:
	sf::Sprite* m_sprite;

	struct TextureInfo
	{
		hash_type m_ResourceName;
		sf::IntRect m_TextureBounds;
	};

	std::array<std::optional<TextureInfo>, 32> m_textures;
	std::map<std::string, size_t> m_textureNameIndexes;

	size_t m_numAssignedTextures;

	bool LoadTextureNode(const XmlNode* node);
};

#endif
