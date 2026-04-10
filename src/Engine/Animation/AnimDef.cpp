#include "AnimDef.h"

#include <fstream>
#include <hoxml.h>
#include <iostream>
#include <SFML/Graphics/Image.hpp>

#include "../Globals.h"
#include "../TextureManager.h"
#include "../CodeGen/Resources.hpp"

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
	std::string line, text;
	std::ifstream in(m_filepath);
	while (std::getline(in, line))
	{
		text += line + "\n";
	}

	if (text.empty())
	{
		return false;
	}

	const char* content = text.c_str();

	const size_t content_length = strlen(content);

	hoxml_context_t* hoxml_context = new hoxml_context_t();
	const auto buffer = static_cast<char*>(malloc(content_length * 2));

	hoxml_init(hoxml_context, buffer, content_length * 2);

	// Loop until the "end of document" code is returned
	hoxml_code_t code = hoxml_parse(hoxml_context, content, content_length);
	while (code != HOXML_END_OF_DOCUMENT)
	{
		if (code == HOXML_ELEMENT_BEGIN)
		{
			if (strcmp("AnimDict", hoxml_context->tag) == 0)
			{
				if (!ParseAnimDict(m_filepath.parent_path(), hoxml_context, content, content_length))
				{
					return false;
				}
			}
		}
		if (code == HOXML_ERROR_TAG_MISMATCH)
		{
			printf("TSX::Init: Start tag did not match end tag on line %u\n", hoxml_context->line);
			return false;
		}

		code = hoxml_parse(hoxml_context, content, content_length);
	}

	delete hoxml_context;
	free(buffer);

	// TODO: Should these have a lifetime?
	ANIMATION_DICTIONARIES[m_name] = *this;
	return true;
}

AnimationDictionary::AnimationDictionary(std::filesystem::path filepath) :
	m_filepath(std::move(filepath))
{
}

bool AnimationDictionary::ParseAnimDict(const std::filesystem::path& parentFolderPath, hoxml_context_t*& context,
										const char* xml, const size_t xmlLength)
{
	hoxml_code_t code = HOXML_ELEMENT_BEGIN;
	while (code != HOXML_END_OF_DOCUMENT)
	{
		if (code == HOXML_ATTRIBUTE && strcmp("name", context->attribute) == 0)
		{
			m_name = context->value;
		}

		if (code == HOXML_ELEMENT_BEGIN)
		{
			if (strcmp("Animation", context->tag) == 0
				&& !ParseAnimation(context, xml, xmlLength))
			{
				std::cerr << "AnimationDictionary::ParseAnimDict: Failed to parse Animation from xml " << m_filepath << "\n";
				return false;
			}

			if (strcmp("Image", context->tag) == 0)
			{
				if (!ParseImage(parentFolderPath, context, xml, xmlLength))
				{
					std::cerr << "AnimationDictionary::ParseImage: Failed to parse Image from xml " << m_filepath << "\n";
					return false;
				}
			}
		}

		if (code == HOXML_ELEMENT_END && strcmp("AnimDict", context->tag) == 0)
		{
			return true;
		}

		code = hoxml_parse(context, xml, xmlLength);
	}

	return true;
}

bool AnimationDictionary::ParseAnimation(hoxml_context_t*& context, const char* xml, const size_t xmlLength)
{
	Animation anim{ };

	hoxml_code_t code = HOXML_ELEMENT_BEGIN;
	while (code != HOXML_END_OF_DOCUMENT)
	{
		if (code == HOXML_ATTRIBUTE)
		{
			if (strcmp("name", context->attribute) == 0)
			{
				anim.m_Name = context->value;
			}
			else if (strcmp("looping", context->attribute) == 0)
			{
				anim.m_IsLooping = strcmp("true", context->value) == 0;
			}
		}
		else if (code == HOXML_ELEMENT_BEGIN)
		{
			if (strcmp("Frame", context->tag) == 0)
			{
				if (!ParseFrame(anim, context, xml, xmlLength))
				{
					std::cerr << "AnimationDictionary::ParseAnimation: Failed to parse Frame from xml " << m_filepath << "\n";
					return false;
				}
			}
			else if (strcmp("OnEnd", context->tag) == 0)
			{
				if (!ParseAnimEnd(anim, context, xml, xmlLength))
				{
					std::cerr << "AnimationDictionary::ParseAnimation: Failed to parse OnEnd from xml " << m_filepath << "\n";
					return false;
				}
			}
		}
		else if (code == HOXML_ELEMENT_END && strcmp("Animation", context->tag) == 0)
		{
			m_animationClips[anim.m_Name] = anim;
			return true;
		}

		code = hoxml_parse(context, xml, xmlLength);
	}

	return false;
}

bool AnimationDictionary::ParseImage(const std::filesystem::path& /*parentFolderPath*/, hoxml_context_t*& context, const char* xml, const size_t xmlLength)
{
	bool setSpritesheet = false;
	bool setX = false;
	bool setY = false;
	bool setWidth = false;
	bool setHeight = false;
	bool hasColourMask = false;
	unsigned colourMask = ~0U;
	std::filesystem::path imagePath;

	hoxml_code_t code = HOXML_ELEMENT_BEGIN;
	while (code != HOXML_END_OF_DOCUMENT)
	{
		if (code == HOXML_ATTRIBUTE && strcmp("source", context->attribute) == 0)
		{
			const char* spriteSheetResourceName = context->value;
			const auto path = GET_TEXTURE_PATH(spriteSheetResourceName);
			if (!std::filesystem::exists(path))
			{
				std::cerr << "AnimationDictionary::ParseAnimDict: Texture with path " << path <<
				"does not exist!\n";
				return false;
			}

			imagePath = path;
			setSpritesheet = true;
		}
		else if (code == HOXML_ATTRIBUTE && strcmp("topLeftX", context->attribute) == 0)
		{
			m_spriteSheetRect.position.x = std::stoi(context->value);
			setX = true;
		}
		else if (code == HOXML_ATTRIBUTE && strcmp("topLeftY", context->attribute) == 0)
		{
			m_spriteSheetRect.position.y = std::stoi(context->value);
			setY = true;
		}
		else if (code == HOXML_ATTRIBUTE && strcmp("regionWidth", context->attribute) == 0)
		{
			m_spriteSheetRect.size.x = std::stoi(context->value);
			setWidth = true;
		}
		else if (code == HOXML_ATTRIBUTE && strcmp("regionHeight", context->attribute) == 0)
		{
			m_spriteSheetRect.size.y = std::stoi(context->value);
			setHeight = true;
		}
		else if (code == HOXML_ATTRIBUTE && strcmp("maskColour", context->attribute) == 0)
		{
			// Convert read colour from hex to an int
			std::stringstream str;
			str << context->value;
			str >> std::hex >> colourMask;

			hasColourMask = true;
		}
		else if (code == HOXML_ELEMENT_END)
		{
			// TODO: Should these default to 0,0?
			if (!setX)
			{
				std::cerr << "No topLeftX set on the source of the Image!\n";
				return false;
			}

			if (!setY)
			{
				std::cerr << "No topLeftY set on the source of the Image!\n";
				return false;
			}

			// TODO: Should these two default to the width and height of the image?
			if (!setWidth)
			{
				std::cerr << "No regionWidth set on the source of the Image!\n";
				return false;
			}

			if (!setHeight)
			{
				std::cerr << "No regionHeight set on the source of the Image!\n";
				return false;
			}

			if (!setSpritesheet)
			{
				std::cerr << "No spritesheet loaded!\n";
				return false;
			}

			const bool success = hasColourMask
									 ? TEXTUREMANAGER.LoadTextureFromImage(GetName(), imagePath, m_spriteSheetRect, colourMask)
									 : TEXTUREMANAGER.LoadTextureFromImage(GetName(), imagePath, m_spriteSheetRect);

			if (!success)
			{
				std::cerr << "AnimationDictionary::ParseImage: TextureManager was unable to load texture with path " << imagePath << "\n";
			}

			return success;
		}


		code = hoxml_parse(context, xml, xmlLength);
	}

	return false;
}

bool AnimationDictionary::ParseFrame(Animation& animation, hoxml_context_t*& context, const char* xml, const size_t xmlLength)
{
	hoxml_code_t code = HOXML_ELEMENT_BEGIN;

	AnimationFrame frame{ };
	bool fullyParsedFrame = false;
	while (!fullyParsedFrame && code != HOXML_END_OF_DOCUMENT)
	{
		if (code == HOXML_ATTRIBUTE)
		{
			if (strcmp("topLeftX", context->attribute) == 0)
			{
				frame.m_TopLeftX = std::stoi(context->value) - m_spriteSheetRect.position.x;
			}
			else if (strcmp("topLeftY", context->attribute) == 0)
			{
				frame.m_TopLeftY = std::stoi(context->value) - m_spriteSheetRect.position.y;
			}
			else if (strcmp("duration", context->attribute) == 0)
			{
				frame.m_DurationMS = std::stoi(context->value);
			}
			else if (strcmp("spriteWidth", context->attribute) == 0)
			{
				frame.m_SpriteWidth = std::stoi(context->value);
			}
			else if (strcmp("spriteHeight", context->attribute) == 0)
			{
				frame.m_SpriteHeight = std::stoi(context->value);
			}
		}
		else if (code == HOXML_ELEMENT_END && strcmp("Frame", context->tag) == 0)
		{
			fullyParsedFrame = true;
		}

		if (!fullyParsedFrame)
		{
			code = hoxml_parse(context, xml, xmlLength);
		}
	}

	if (!fullyParsedFrame)
	{
		std::cerr << "AnimationDictionary::ParseFrame: Reached the end of the Frame element without fully parsing attributes! " << m_filepath << "\n";
		return false;
	}

	animation.m_Frames.push_back(frame);
	return true;
}

bool AnimationDictionary::ParseAnimEnd(Animation& animation,
									   hoxml_context_t*& context, const char* xml, size_t xmlLength)
{
	hoxml_code_t code = HOXML_ELEMENT_BEGIN;

	while (code != HOXML_END_OF_DOCUMENT)
	{
		if (code == HOXML_ATTRIBUTE && strcmp("animName", context->attribute) == 0)
		{
			animation.m_OnAnimEnd = context->value;
			return true;
		}
		code = hoxml_parse(context, xml, xmlLength);
	}

	return false;
}
