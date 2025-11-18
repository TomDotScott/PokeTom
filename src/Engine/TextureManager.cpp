#include "TextureManager.h"

#include <iostream>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>


TextureManager& TextureManager::Get()
{
	static TextureManager* textureManager;
	if (textureManager == nullptr)
	{
		textureManager = new TextureManager();
	}

	return *textureManager;
}

bool TextureManager::LoadTextureFromImage(const std::string& name, const std::filesystem::path& path, uint32_t maskColour)
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

	const auto texture = new sf::Texture(image);
	m_textures[name] = texture;
	return true;
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

	const auto texture = new sf::Texture(path);

	m_textures[name] = texture;

	return true;
}

const sf::Texture* TextureManager::GetTexture(const std::string& name) const
{
	if (m_textures.find(name) == m_textures.end())
	{
		printf("Texture %s does not exist\n", name.c_str());
		return nullptr;
	}

	return m_textures.at(name);
}

TextureManager::~TextureManager()
{
	for (auto& [name, texture] : m_textures)
	{
		delete texture;
	}
}
