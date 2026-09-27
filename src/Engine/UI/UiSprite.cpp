#include "UiSprite.h"

#include <iostream>

#include "../Asserts.h"
#include "../TextureManager.h"
#include "../CodeGen/Resources.hpp"

UiSprite::UiSprite(UiElement* parent) :
	UiElement(eType::Sprite, parent),
	m_sprite(nullptr),
	m_textures({ std::nullopt }),
	m_numAssignedTextures(0)
{
}

void UiSprite::SetElementPosition(const sf::Vector2f& position)
{
	m_absolutePosition = position;

	SetPosition(position + CalculateOffsetFromParents());

	RecalculatePositionAfterParentMoved();
}

void UiSprite::RecalculatePositionAfterParentMoved()
{
	if (m_sprite)
	{
		m_sprite->setPosition(m_absolutePosition + CalculateOffsetFromParents());
	}
}

void UiSprite::SetScale(const sf::Vector2f& scale) const
{
	m_sprite->setScale(scale);
}

sf::Vector2f UiSprite::GetSize() const
{
	return m_sprite->getGlobalBounds().size;
}

bool UiSprite::LoadFromXML(const XmlNode& node)
{
	if (!UiElement::LoadFromXML(node))
	{
		return false;
	}

	const auto* texturesNode = node.Child("Textures");
	ASSERT(texturesNode);

	if (texturesNode == nullptr)
	{
		return false;
	}

	for (const auto& textureNode : texturesNode->Children("texture"))
	{
		const bool loaded = LoadTextureNode(textureNode);
		ASSERT(loaded, "Texture failed to load. See the logs for more info");

		if (!loaded)
		{
			return false;
		}
	}

	const auto& firstTextureInfo = m_textures[0];

	ASSERT(firstTextureInfo.has_value(), "Something has gone wrong. The first texture in the list was not loaded properly!");

	const sf::Texture* texture = TEXTUREMANAGER.GetTexture(firstTextureInfo.value().m_ResourceName);

	// TODO: This probably doesn't have to be heap allocated!
	m_sprite = new sf::Sprite(*texture);

	m_sprite->setTextureRect(firstTextureInfo.value().m_TextureBounds);

	sf::Vector2f scale{ 1, 1 };
	const auto* scaleNode = node.Child("scale");
	if (scaleNode != nullptr)
	{
		scale = scaleNode->Attr("x", "y", { 1, 1 });
	}

	ASSERT(m_sprite);

	m_sprite->setPosition(GetPosition());
	SetScale(scale);
	AddDrawable(m_sprite);
	return true;
}

void UiSprite::SetTexture(const size_t textureIndex)
{
	ASSERT(textureIndex < 32);

	std::optional<TextureInfo>& textureInfoOpt = m_textures[textureIndex];
	ASSERT(textureInfoOpt.has_value());
	TextureInfo& textureInfo = textureInfoOpt.value();


	const sf::Texture* texture = TEXTUREMANAGER.GetTexture(textureInfo.m_ResourceName);
	m_sprite->setTexture(*texture, true);
	m_sprite->setTextureRect(textureInfo.m_TextureBounds);
}

void UiSprite::SetTexture(const std::string& textureID)
{
	ASSERT(m_textureNameIndexes.contains(textureID));
	SetTexture(m_textureNameIndexes.at(textureID));
}

bool UiSprite::LoadTextureNode(const XmlNode* node)
{
	const std::string resourceID = node->Attr("textureResourceName", std::string{ "" });

	ASSERT(!resourceID.empty());
	if (resourceID.empty())
	{
		return false;
	}

	const std::filesystem::path texturePath = GET_TEXTURE_PATH(resourceID);
	if (!std::filesystem::exists(texturePath))
	{
		ASSERT(false, "Path : %s does not exist!", texturePath.c_str());
		return false;
	}

	// Load the texture into the TextureManager if it's not there already
	if (!TEXTUREMANAGER.LoadTexture(HASH(resourceID), texturePath))
	{
		ASSERT(false, "Failed to load texture : %s", texturePath.c_str());
		return false;
	}

	m_textures[m_numAssignedTextures] = TextureInfo{};
	TextureInfo& currentTexture = m_textures[m_numAssignedTextures].value();
	currentTexture.m_ResourceName = HASH(resourceID.c_str());

	const auto topLeftX = node->Attr("topLeftX", ~0U);
	const auto topLeftY = node->Attr("topLeftY", ~0U);
	const auto width = node->Attr("width", ~0U);
	const auto height = node->Attr("height", ~0U);
	if (topLeftX ^ topLeftY ^ width ^ height)
	{
		currentTexture.m_TextureBounds.position = sf::Vector2i{ static_cast<int>(topLeftX), static_cast<int>(topLeftY) };
		currentTexture.m_TextureBounds.size = sf::Vector2i{ static_cast<int>(width), static_cast<int>(height) };
	}

	const std::string textureID = node->Attr("id", std::string{ "" });
	if (!textureID.empty())
	{
		m_textureNameIndexes[textureID] = m_numAssignedTextures;
	}

	m_numAssignedTextures++;
	return true;
}
