#include "UiSprite.h"

#include <iostream>

#include "../Globals.h"
#include "../TextureManager.h"

UiSprite::UiSprite() :
	UiElement(eType::Sprite),
	m_sprite(nullptr),
	m_screenScaleFactor(1.f, 1.f),
	m_scaleFactorFromXml(1.f, 1.f)
{
}

void UiSprite::SetElementPosition(const sf::Vector2f& position)
{
	SetPosition(position);

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

bool UiSprite::LoadFromXML(const XmlNode& node)
{
	if (UiElement::LoadFromXML(node))
	{
		return false;
	}

	const auto* textureNode = node.Child("Texture");
	if (textureNode == nullptr)
	{
		return false;
	}

	if (!LoadTexture(textureNode->m_Content))
	{
		return false;
	}

	const auto* scaleNode = node.Child("scale");
	if (scaleNode != nullptr)
	{
		m_scaleFactorFromXml = scaleNode->Attr("x", "y", { 1, 1 });
	}

	const auto* layerNode = node.Child("layer");
	if (layerNode == nullptr)
	{
		std::cerr << "UiSprite::LoadFromXML - Warning, no layer provided for UiSprite " << GetName() <<
			"! Setting to foreground";
		SetLayer(eLayer::FOREGROUND);
	}
	else
	{
		if (layerNode->m_Content == "foreground")
		{
			SetLayer(eLayer::FOREGROUND);
		}
		else if (layerNode->m_Content == "midground")
		{
			SetLayer(eLayer::MIDGROUND);
		}
		else if (layerNode->m_Content == "background")
		{
			SetLayer(eLayer::BACKGROUND);
		}
		else
		{
			std::cerr << "UiSprite::LoadFromXML - Unknown layer provided for UiSprite " << GetName() <<
				layerNode->m_Content << "\n";
			return false;
		}
	}

	m_sprite->setPosition(m_position);
	SetScale(m_scaleFactorFromXml);
	AddDrawable(m_sprite);
	return true;
}

bool UiSprite::LoadTexture(const std::filesystem::path& filePath)
{
	// Grab the texture name
	if (!exists(filePath))
	{
		printf(" Texture: %ls does not exist!\n", filePath.c_str());
		return false;
	}

	const std::string textureName = filePath.stem().string();

	// Load the texture into the TextureManager
	if (!TEXTUREMANAGER.LoadTexture(textureName, filePath))
	{
		printf(" Sprite: Failed to load texture: %ls", filePath.c_str());
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
