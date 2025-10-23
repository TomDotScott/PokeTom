#include "UiSprite.h"
#include "../Globals.h"
#include "../TextureManager.h"

UiSprite::UiSprite() :
	UiElement(eType::Sprite),
	m_sprite(nullptr),
	m_screenScaleFactor(1.f, 1.f),
	m_scaleFactorFromXml(1.f, 1.f)
{
}

void UiSprite::SetPosition(const sf::Vector2f& position)
{
	GameObject::SetPosition(position);

	m_sprite->setPosition(m_position);
}

void UiSprite::SetScale(const sf::Vector2f& scale) const
{
	const sf::Vector2f overallScale{ m_screenScaleFactor.x * scale.y, m_screenScaleFactor.x * scale.y };
	m_sprite->setScale(overallScale);
}

sf::Vector2f UiSprite::GetSize() const
{
	return m_sprite->getGlobalBounds().size;
}

bool UiSprite::ParseEndElement(hoxml_context_t*& context)
{
	if (!context)
	{
		return false;
	}

	if (context->tag && strcmp("Sprite", context->tag) == 0)
	{
		if (!m_sprite)
		{
			printf(" Closing tag found and no sprite was assigned to %s!\n", GetName().c_str());
			return false;
		}
		m_sprite->setPosition(m_position);

		SetScale(m_scaleFactorFromXml);

		AddDrawable(m_sprite);

		// When we have finished assigning the sprite, we can return true!
		return true;
	}

	if (context->content == nullptr || OnlyWhitespace(context->content))
	{
		return false;
	}

	if (strcmp("layer", context->tag) == 0)
	{
		if (strcmp("foreground", context->content) == 0)
		{
			SetLayer(eLayer::FOREGROUND);
		}
		else if (strcmp("midground", context->content) == 0)
		{
			SetLayer(eLayer::MIDGROUND);
		}
		else if (strcmp("background", context->content) == 0)
		{
			SetLayer(eLayer::BACKGROUND);
		}
		else
		{
			printf(" Unknown layer: %s!\n", context->content);
		}

		return false;
	}

	if (strcmp("texture", context->tag) == 0)
	{
		const std::filesystem::path filePath = std::filesystem::u8path(context->content);

		// Grab the texture name
		if (!std::filesystem::exists(filePath))
		{
			printf(" Texture: %s does not exist!\n", context->content);
			return false;
		}

		const std::string textureName = filePath.stem().string();

		// Load the texture into the TextureManager
		if (!TEXTUREMANAGER.LoadTexture(textureName, filePath))
		{
			printf(" Sprite: Failed to load texture: %s", context->content);
			return false;
		}

		const sf::Texture* texture = TEXTUREMANAGER.GetTexture(textureName);

		const sf::Vector2f scaleFactor = {
			static_cast<float>(texture->getSize().x) / TRANSFORMED_SCALAR(texture->getSize().x),
			static_cast<float>(texture->getSize().y) / TRANSFORMED_SCALAR(texture->getSize().y)
		};

		m_screenScaleFactor = { 1.f / scaleFactor.x, 1.f / scaleFactor.y };

		m_sprite = new sf::Sprite(*texture);

		return false;
	}

	return UiElement::ParseEndElement(context);
}

bool UiSprite::ParseAttribute(hoxml_context_t*& context)
{
	if (strcmp("scale", context->tag) == 0)
	{
		return ParseVectorAttribute(context, m_scaleFactorFromXml);
	}

	return UiElement::ParseAttribute(context);
}
