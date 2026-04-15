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

sf::Vector2f XmlNode::Attr(const std::string_view& xKey, const std::string_view& yKey) const
{
	return {
		Attr(xKey, 0.f),
		Attr(yKey, 0.f)
	};
}

const XmlNode* XmlNode::Child(const std::string_view& tag) const
{
	for (const auto& child : m_Children)
	{
		if (child.m_Tag == tag)
		{
			return &child;
		}
	}

	return nullptr;
}

std::vector<const XmlNode*> XmlNode::Children(const std::string_view& tag) const
{
	std::vector<const XmlNode*> res;
	for (const auto& child : m_Children)
	{
		if (child.m_Tag == tag)
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
