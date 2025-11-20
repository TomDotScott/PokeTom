#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H
#include <set>
#include <unordered_map>
#include <SFML/Graphics/Texture.hpp>

class TextureManager
{
public:
	static TextureManager& Get();

	bool LoadTexture(std::string_view name, const std::filesystem::path& path);
	bool LoadTexture(const std::string& name, const std::filesystem::path& path);
	bool LoadTextureFromImage(const std::string& name, const std::filesystem::path& path, uint32_t maskColour);

	bool HasTextureLoaded(const std::string& name) const;
	bool HasTextureLoaded(const std::filesystem::path& path) const;

	const std::string& GetTextureNameFromPath(const std::filesystem::path& path) const;

	const sf::Texture* GetTexture(const std::string& name) const;

	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;
	TextureManager(TextureManager&&) = delete;
	TextureManager& operator=(TextureManager&&) = delete;

private:
	std::unordered_map<std::string, sf::Texture> m_textures;
	std::unordered_map<std::string, std::string> m_registeredPaths;

	void RegisterPath(const std::filesystem::path& relativePath, const std::string& resourceName);

	TextureManager();
	~TextureManager() = default;
};

#define TEXTUREMANAGER TextureManager::Get()

#endif
