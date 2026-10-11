#ifndef ANIMDEF_H
#define ANIMDEF_H
#include <string>
#include <unordered_map>
#include <SFML/Graphics/Texture.hpp>

#include "../Factory.h"
#include "../Hash.h"
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
	enum eSpriteAnchor
	{
		TOP_LEFT = 1 << 0,
		TOP_MIDDLE = 1 << 1,
		TOP_RIGHT = 1 << 2,

		MIDDLE_LEFT = 1 << 3,
		MIDDLE_MIDDLE = 1 << 4,
		MIDDLE_RIGHT = 1 << 5,

		BOTTOM_LEFT = 1 << 6,
		BOTTOM_MIDDLE = 1 << 7,
		BOTTOM_RIGHT = 1 << 8,
	};

	hash_type m_Name;
	bool m_IsLooping;
	std::vector<AnimationFrame> m_Frames;
	bool m_HasOnAnimEnd;
	hash_type m_OnAnimEnd;
	eSpriteAnchor m_SpriteAnchor = MIDDLE_MIDDLE;
};

class AnimationDictionary : public ISerialisable
{
public:
	AnimationDictionary() = default;

	const Animation& GetClip(const hash_type& name) const;

	const hash_type& GetName() const;
	bool HasClip(const hash_type& name) const;

	static std::shared_ptr<AnimationDictionary> Create(const std::filesystem::path& path);
	static std::shared_ptr<AnimationDictionary> Create(const XmlNode& root);

private:
	hash_type m_name;
	sf::IntRect m_spriteSheetRect;
	std::unordered_map<hash_type, Animation> m_animationClips;


	bool LoadFromXML(const XmlNode& node) override;
};

#endif
