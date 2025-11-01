#ifndef ANIMDEF_H
#define ANIMDEF_H
#include <hoxml.h>
#include <string>
#include <unordered_map>
#include <SFML/Graphics/Texture.hpp>

#include "../Factory.h"

struct AnimationFrame
{
	uint32_t m_TopLeftX;
	uint32_t m_TopLeftY;
	uint32_t m_DurationMS;
	uint32_t m_SpriteWidth;
	uint32_t m_SpriteHeight;
};

struct Animation
{
	std::string m_Name;
	bool m_IsLooping;
	std::vector<AnimationFrame> m_Frames;
	std::string m_OnAnimEnd;
};

class AnimationDictionary : public Factory<AnimationDictionary>
{
public:
	friend class Factory<AnimationDictionary>;

	const Animation& GetClip(const std::string& name) const;

	AnimationDictionary() = default;

protected:
	bool Init() override;

private:
	std::string m_name;
	std::filesystem::path m_filepath;
	sf::Texture m_spriteSheet;
	uint32_t m_spriteWidth;
	uint32_t m_spriteHeight;
	std::unordered_map<std::string, Animation> m_animationClips;

	AnimationDictionary(std::filesystem::path filepath);
	bool ParseAnimDict(const std::filesystem::path& parentFolderPath, hoxml_context_t*& context, const char* xml, size_t xmlLength);
	bool ParseAnimation(hoxml_context_t*& context, const char* xml, size_t xmlLength);

	bool ParseFrame(Animation& animation, hoxml_context_t*& context, const char* xml, size_t xmlLength);
	bool ParseAnimEnd(Animation& animation, hoxml_context_t*& context, const char* xml, size_t xmlLength);
};

static inline std::unordered_map<std::string, AnimationDictionary> ANIMATION_DICTIONARIES;

#endif
