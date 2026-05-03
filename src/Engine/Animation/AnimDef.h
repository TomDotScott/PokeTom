#ifndef ANIMDEF_H
#define ANIMDEF_H
#include <string>
#include <unordered_map>
#include <SFML/Graphics/Texture.hpp>

#include "../Factory.h"
#include "../ISerialisable.h"

struct AnimationFrame
{
	uint32_t m_TopLeftX;
	uint32_t m_TopLeftY;
	uint32_t m_DurationMS;
	uint32_t m_SpriteWidth;
	uint32_t m_SpriteHeight;
	bool m_SpriteFlippedHorizontal = false;
	bool m_SpriteFlippedVertical = false;
};

struct Animation
{
	std::string m_Name;
	bool m_IsLooping;
	std::vector<AnimationFrame> m_Frames;
	std::string m_OnAnimEnd;
};

class AnimationDictionary : public Factory<AnimationDictionary>, public ISerialisable
{
public:
	friend class Factory<AnimationDictionary>;

	const Animation& GetClip(const std::string& name) const;

	// TODO: Get rid of this - I think we need a wrapper for the map below...
	AnimationDictionary() = default;

	const std::string& GetName() const;

protected:
	bool Init() override;

private:
	std::string m_name;
	std::filesystem::path m_filepath;
	sf::IntRect m_spriteSheetRect;
	std::unordered_map<std::string, Animation> m_animationClips;

	AnimationDictionary(std::filesystem::path filepath);

	bool LoadFromXML(const XmlNode& node) override;
};

static inline std::unordered_map<std::string, AnimationDictionary> ANIMATION_DICTIONARIES;

#endif
