#include "TextureManager.h"

#include <iostream>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>

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

bool TextureManager::LoadTextureFromImage(const std::string& name, const std::filesystem::path& path,
	const uint32_t maskColour)
{
	if (!std::filesystem::exists(path))
	{
		std::cerr << "The file " << path << " does not exist\n";
		return false;
	}

	sf::Image image;
	if (!image.loadFromFile(path))
	{
		std::cerr << "Unable to load texture into sf::Image from path " << path << "\n";
	}

	image.createMaskFromColor(sf::Color{ maskColour });

	m_textures[name] = sf::Texture(image);
	RegisterPath(path, name);
	return true;
}

bool TextureManager::HasTextureLoaded(const std::string& name) const
{
	return m_textures.find(name) != m_textures.end();
}

bool TextureManager::HasTextureLoaded(const std::filesystem::path& path) const
{
	return m_registeredPaths.find(path.string()) != m_registeredPaths.end();
}

const std::string& TextureManager::GetTextureNameFromPath(const std::filesystem::path& path) const
{
	if (!HasTextureLoaded(path))
	{
		std::cerr << "TextureManager::GetTextureNameFromPath - TEXTURE AT PATH " << path << "HAS NOT BEEN LOADED! ERRORS WILL FOLLOW!\n";
	}

	return m_registeredPaths.at(path.string());
}

bool TextureManager::LoadTexture(const std::string_view name, const std::filesystem::path& path)
{
	return LoadTexture(std::string{ name }, path);
}

bool TextureManager::LoadTexture(const std::string& name, const std::filesystem::path& path)
{
	if (m_textures.find(name) != m_textures.end())
	{
		printf("A texture with the name: %s already exists!\n", name.c_str());
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

const sf::Texture* TextureManager::GetTexture(const std::string& name) const
{
	if (m_textures.find(name) == m_textures.end())
	{
		printf("Texture %s does not exist\n", name.c_str());
		return nullptr;
	}

	return &m_textures.at(name);
}

void TextureManager::RegisterPath(const std::filesystem::path& relativePath, const std::string& resourceName)
{
	const auto combinedPaths = (std::filesystem::current_path() / relativePath).lexically_normal();
	m_registeredPaths[combinedPaths.string()] = resourceName;
}

TextureManager::TextureManager()
{
	for (const auto& [resourceID, resourcePath] : textures_resources)
	{
		if (!LoadTexture(resourceID, resourcePath))
		{
			std::cout << "TextureManager::TextureManager - Failed to load texture " << resourceID << " at path " << resourcePath << "\n";
		}
	}
}
