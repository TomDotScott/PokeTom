#include "AnimDef.h"

#include <fstream>
#include <hoxml.h>
#include <iostream>

#include "../Globals.h"

const Animation& AnimationDictionary::GetClip(const std::string& name) const
{
	return m_animationClips.at(name);
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
#if BUILD_DEBUG
			printf("AnimationDictionary::Init: Opened <%s>\n", hoxml_context->tag);
#endif

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

	free(buffer);

	// TODO: Should these have a lifetime?
	ANIMATION_DICTIONARIES[m_name] = *this;
	return true;
}

AnimationDictionary::AnimationDictionary(std::filesystem::path filepath) :
	m_filepath(std::move(filepath)),
	m_spriteWidth(0),
	m_spriteHeight(0)
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
				const std::string combinedPaths = CombinePaths(parentFolderPath.string(), context->value);

				if (!std::filesystem::exists(combinedPaths))
				{
					std::cerr << "AnimationDictionary::ParseAnimDict: Texture with path " << combinedPaths <<
						"does not exist!\n";
					return false;
				}

				// TODO: Think about how to integrate this with the TextureManager...
				if (!m_spriteSheet.loadFromFile(combinedPaths))
				{
					std::cerr << "AnimationDictionary::ParseAnimDict: Failed to load texture from path " <<
						combinedPaths << "\n";
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
	Animation anim{};

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

bool AnimationDictionary::ParseFrame(Animation& animation, hoxml_context_t*& context, const char* xml, const size_t xmlLength)
{
	hoxml_code_t code = HOXML_ELEMENT_BEGIN;

	AnimationFrame frame{};
	bool fullyParsedFrame = false;
	while (!fullyParsedFrame && code != HOXML_END_OF_DOCUMENT)
	{
		if (code == HOXML_ATTRIBUTE)
		{
			if (strcmp("topLeftX", context->attribute) == 0)
			{
				frame.m_TopLeftX = std::stoi(context->value);
			}
			else if (strcmp("topLeftY", context->attribute) == 0)
			{
				frame.m_TopLeftY = std::stoi(context->value);
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
