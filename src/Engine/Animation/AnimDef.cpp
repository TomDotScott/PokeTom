#include "AnimDef.h"

#include <iostream>
#include <SFML/Graphics/Image.hpp>

#include "../Globals.h"
#include "../TextureManager.h"
#include "../CodeGen/Resources.hpp"
#include "../XML/XmlDocument.h"

const Animation& AnimationDictionary::GetClip(const std::string& name) const
{
	return m_animationClips.at(name);
}

const std::string& AnimationDictionary::GetName() const
{
	return m_name;
}

bool AnimationDictionary::Init()
{
	XmlDocument doc;
	doc.Load(m_filepath);

	const auto& root = doc.Root().Child("AnimDict");

	if (!LoadFromXML(*root))
	{
		return false;
	}

	// TODO: Should these have a lifetime?
	ANIMATION_DICTIONARIES[m_name] = *this;
	return true;
}

AnimationDictionary::AnimationDictionary(std::filesystem::path filepath) :
	m_filepath(std::move(filepath))
{
}

bool AnimationDictionary::LoadFromXML(const XmlNode& node)
{
	m_name = node.Attr("name", std::string{ "" });
	if (m_name.empty())
	{
		return false;
	}

	// Load the Texture so we can calculate the correct offsets for the animation frames
	const auto imageTag = node.Child("Image");
	if (imageTag == nullptr)
	{
		return false;
	}

	std::string_view imagePath = GET_TEXTURE_PATH(imageTag->Attr("source", std::string{ "" }));

	if (imagePath.empty() || !std::filesystem::exists(imagePath))
	{
		std::cerr << "AnimationDictionary::ParseAnimDict: Texture with path " << imagePath <<
			" does not exist!\n";
		return false;
	}

	m_spriteSheetRect = {
		{
			imageTag->Attr("topLeftX", 0),
			imageTag->Attr("topLeftY", 0)
		},
		{
			imageTag->Attr("regionWidth", 0),
			imageTag->Attr("regionHeight", 0)
		}
	};

	unsigned colourMask = ~0U;
	std::string maskColour = imageTag->Attr("maskColour", std::string{ "" });
	const bool hasColourMask = !maskColour.empty();

	if (hasColourMask)
	{
		// Convert read colour from hex to an int
		std::stringstream str;
		str << maskColour;
		str >> std::hex >> colourMask;
	}

	const bool success = hasColourMask
		                     ? TEXTUREMANAGER.LoadTextureFromImage(GetName(), imagePath, m_spriteSheetRect, colourMask)
		                     : TEXTUREMANAGER.LoadTextureFromImage(GetName(), imagePath, m_spriteSheetRect);

	if (!success)
	{
		std::cerr << "AnimationDictionary::ParseImage: TextureManager was unable to load texture with path " <<
			imagePath << "\n";
		return false;
	}

	const std::vector<const XmlNode*> animations = node.Children("Animation");

	for (const auto& animation : animations)
	{
		Animation anim;

		anim.m_Name = animation->Attr("name", std::string{ "" });
		anim.m_IsLooping = animation->Attr("looping", false);

		const auto frameNodes = animation->Children("Frame");
		std::vector<AnimationFrame> frames;
		frames.reserve(frameNodes.size());

		for (const auto& frameNode : frameNodes)
		{
			frames.emplace_back(
				frameNode->Attr("topLeftX", 0U) - m_spriteSheetRect.position.x,
				frameNode->Attr("topLeftY", 0U) - m_spriteSheetRect.position.y,
				frameNode->Attr("duration", 0U),
				frameNode->Attr("spriteWidth", 0U),
				frameNode->Attr("spriteHeight", 0U)
			);
		}

		anim.m_Frames = frames;

		const auto onAnimEnd = animation->Child("OnEnd");
		if (onAnimEnd != nullptr)
		{
			anim.m_OnAnimEnd = onAnimEnd->Attr("animName", std::string{ "" });
		}

		if (anim.m_Name.empty())
		{
			return false;
		}

		m_animationClips[anim.m_Name] = anim;
	}

	return true;
}
