#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H
#include <set>
#include <unordered_map>
#include <SFML/Graphics/Texture.hpp>

#include "Hash.h"

class TextureManager
{
public:
	static TextureManager& Get();

	bool LoadTexture(hash_type name, const std::filesystem::path& path);
	bool LoadTextureFromImage(hash_type name, const std::filesystem::path& path, sf::IntRect region);
	bool LoadTextureFromImage(hash_type name, const std::filesystem::path& path, sf::IntRect region, uint32_t maskColour);

	bool HasTextureLoaded(hash_type name) const;
	bool HasTextureLoaded(const std::filesystem::path& path) const;

	const hash_type& GetTextureNameFromPath(const std::filesystem::path& path) const;

	const sf::Texture* GetTexture(hash_type name) const;

	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;
	TextureManager(TextureManager&&) = delete;
	TextureManager& operator=(TextureManager&&) = delete;

private:
	// Key = Texture ID, Value = Texture
	std::unordered_map<hash_type, sf::Texture> m_textures;

	// Key = Texture Path, Value = Texture ID
	std::unordered_map<std::string, hash_type > m_registeredPaths;

	void RegisterPath(const std::filesystem::path& relativePath, hash_type resourceName);

	std::shared_ptr<sf::Image> LoadImage(const std::filesystem::path& path);

	TextureManager();
	~TextureManager() = default;
};

#define TEXTUREMANAGER TextureManager::Get()

#endif
