#include "TextureManager.h"


TextureManager& TextureManager::Get()
{
	static TextureManager* textureManager;
	if (textureManager == nullptr)
	{
		textureManager = new TextureManager();
	}

	return *textureManager;
}

bool TextureManager::LoadTexture(const std::string& name, const std::filesystem::path& path)
{
	if (m_textures.find(name) != m_textures.end())
	{
		printf("A texture with the name: %s already exists!\n", name.c_str());
		return false;
	}

	if (!std::filesystem::exists(path))
	{
		printf("The file %ls does not exist\n", path.c_str());
		return false;
	}

	sf::Texture* texture = new sf::Texture(path);

	m_textures[name] = texture;

	return true;
}

sf::Texture* TextureManager::GetTexture(const std::string& name) const
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
		texture = nullptr;
	}
}
