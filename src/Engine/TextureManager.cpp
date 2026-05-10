#include "TextureManager.h"

#include <iostream>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>

#include "Asserts.h"
#include "CodeGen/Resources.hpp"


TextureManager& TextureManager::Get()
{
	static TextureManager* textureManager;
	if (textureManager == nullptr)
	{
		textureManager = new TextureManager();
	}

	return *textureManager;
}

std::shared_ptr<sf::Image> TextureManager::LoadImage(const std::filesystem::path& path)
{
	if (!std::filesystem::exists(path))
	{
		std::cerr << "The file " << path << " does not exist\n";
		return nullptr;
	}

	sf::Image image;
	if (!image.loadFromFile(path))
	{
		std::cerr << "Unable to load texture into sf::Image from path " << path << "\n";
		return nullptr;
	}

	return std::make_shared<sf::Image>(image);
}

bool TextureManager::LoadTextureFromImage(hash_type name, const std::filesystem::path& path, const sf::IntRect region)
{
	const std::shared_ptr<sf::Image> image = LoadImage(path);
	if (image == nullptr)
	{
		return false;
	}

	m_textures[name] = sf::Texture(*image, false, region);
	RegisterPath(path, name);
	return true;
}

bool TextureManager::LoadTextureFromImage(hash_type name, const std::filesystem::path& path, const sf::IntRect region, const uint32_t maskColour)
{
	const std::shared_ptr<sf::Image> image = LoadImage(path);
	if (image == nullptr)
	{
		return false;
	}

	image->createMaskFromColor(sf::Color{ maskColour });

	m_textures[name] = sf::Texture(*image, false, region);
	RegisterPath(path, name);
	return true;
}

bool TextureManager::HasTextureLoaded(hash_type name) const
{
	return m_textures.contains(name);
}

bool TextureManager::HasTextureLoaded(const std::filesystem::path& path) const
{
	return m_registeredPaths.contains(path.string());
}

const hash_type& TextureManager::GetTextureNameFromPath(const std::filesystem::path& path) const
{
	if (!HasTextureLoaded(path))
	{
		std::cerr << "TextureManager::GetTextureNameFromPath - TEXTURE AT PATH " << path << "HAS NOT BEEN LOADED! ERRORS WILL FOLLOW!\n";
	}

	return m_registeredPaths.at(path.string());
}

bool TextureManager::LoadTexture(hash_type name, const std::filesystem::path& path)
{
	if (m_textures.contains(name))
	{
		return true;
	}

	if (!std::filesystem::exists(path))
	{
		printf("The file %ls does not exist\n", path.c_str());
		return false;
	}

	m_textures[name] = sf::Texture(path);
	RegisterPath(path, name);
	return true;
}

const sf::Texture* TextureManager::GetTexture(hash_type name) const
{
	ASSERT(m_textures.contains(name));
	return &m_textures.at(name);
}

void TextureManager::RegisterPath(const std::filesystem::path& relativePath, hash_type resourceName)
{
	const auto combinedPaths = (std::filesystem::current_path() / relativePath).lexically_normal();
	m_registeredPaths[combinedPaths.string()] = resourceName;
}

TextureManager::TextureManager()
{
	for (const auto& [resourceID, resourcePath] : textures_resources)
	{
		if (!LoadTexture(static_cast<hash_type>(HASH(resourceID)), resourcePath))
		{
			std::cout << "TextureManager::TextureManager - Failed to load texture " << resourceID << " at path " << resourcePath << "\n";
		}
	}
}
