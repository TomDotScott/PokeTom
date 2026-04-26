#include "XmlNode.h"

#include <algorithm>

std::string XmlNode::Attr(const std::string_view& key, const std::string& fallback) const
{
	const auto it = m_Attributes.find(std::string{ key });
	return it == m_Attributes.end() ? fallback : it->second;
}

int XmlNode::Attr(const std::string_view& key, const int fallback) const
{
	const auto it = m_Attributes.find(std::string{ key });
	if (it == m_Attributes.end())
	{
		return fallback;
	}

	try
	{
		return std::stoi(it->second);
	}
	catch (...)
	{
		return fallback;
	}
}

unsigned XmlNode::Attr(const std::string_view& key, const unsigned fallback) const
{
	const auto it = m_Attributes.find(std::string{ key });
	if (it == m_Attributes.end())
	{
		return fallback;
	}

	try
	{
		return std::stoul(it->second);
	}
	catch (...)
	{
		return fallback;
	}
}

float XmlNode::Attr(const std::string_view& key, const float fallback) const
{
	const auto it = m_Attributes.find(std::string{ key });
	if (it == m_Attributes.end())
	{
		return fallback;
	}

	try
	{
		return std::stof(it->second);
	}
	catch (...)
	{
		return fallback;
	}
}

bool XmlNode::Attr(const std::string_view& key, const bool fallback) const
{
	const auto it = m_Attributes.find(std::string{ key });
	if (it == m_Attributes.end())
	{
		return fallback;
	}

	std::string value = it->second;
	std::ranges::transform(value, value.begin(),
	                       [](const unsigned char c)
	                       {
		                       return std::tolower(c);
	                       });

	if (value == "true")
	{
		return true;
	}

	if (value == "false")
	{
		return false;
	}

	return fallback;
}

sf::Vector2f XmlNode::Attr(const std::string_view& xKey, const std::string_view& yKey, sf::Vector2f fallback) const
{
	constexpr float MAX_FLOAT = std::numeric_limits<float>::max();
	const float x = Attr(xKey, MAX_FLOAT);
	const float y = Attr(yKey, MAX_FLOAT);

	if (x == MAX_FLOAT || y == MAX_FLOAT)
	{
		return fallback;
	}

	return {
		x,
		y
	};
}

const XmlNode* XmlNode::Child(const std::string_view& tag) const
{
	auto toLower = [](const unsigned char c) { return std::tolower(c); };

	std::string tagToLower{ tag };
	std::ranges::transform(tagToLower, tagToLower.begin(),
	                       toLower);

	for (const auto& child : m_Children)
	{
		std::string lowerName{ child.m_Tag };
		std::ranges::transform(lowerName, lowerName.begin(),
		                       toLower);


		if (lowerName == tagToLower)
		{
			return &child;
		}
	}

	return nullptr;
}

std::vector<const XmlNode*> XmlNode::Children() const
{
	std::vector<const XmlNode*> res;
	res.reserve(m_Children.size());

	for (const auto& c : m_Children)
	{
		res.emplace_back(&c);
	}

	return res;
}

std::vector<const XmlNode*> XmlNode::Children(const std::string_view& tag) const
{
	auto toLower = [](const unsigned char c) { return std::tolower(c); };

	std::string lowerSearchTag{ tag };
	std::ranges::transform(lowerSearchTag, lowerSearchTag.begin(),
		toLower);

	std::vector<const XmlNode*> res;
	res.reserve(m_Children.size());

	for (const auto& child : m_Children)
	{
		std::string currentTagLower{ child.m_Tag };
		std::ranges::transform(currentTagLower, currentTagLower.begin(),
			toLower);

		if (lowerSearchTag == currentTagLower)
		{
			res.emplace_back(&child);
		}
	}

	return res;
}

bool XmlNode::HasChild(const std::string_view& tag) const
{
	return Child(tag) != nullptr;
}
