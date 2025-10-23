#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H
#include <unordered_map>
#include <SFML/Graphics/Texture.hpp>

class TextureManager
{
public:
	static TextureManager& Get();

	bool LoadTexture(const std::string& name, const std::filesystem::path& path);

	sf::Texture* GetTexture(const std::string& name) const;

private:
	std::unordered_map<std::string, sf::Texture*> m_textures;

	TextureManager() = default;
	~TextureManager();
};

#define TEXTUREMANAGER TextureManager::Get()

#endif
