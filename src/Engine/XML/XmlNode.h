#ifndef XMLNODE_H
#define XMLNODE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <SFML/System/Vector2.hpp>

struct XmlNode
{
	std::string Attr(const std::string_view& key, const std::string& fallback = "") const;
	int Attr(const std::string_view& key, int fallback = 0) const;
	unsigned Attr(const std::string_view& key, unsigned fallback = 0U) const;
	float Attr(const std::string_view& key, float fallback = 0.0f) const;
	bool Attr(const std::string_view& key, bool fallback = false) const;
	sf::Vector2f Attr(const std::string_view& xKey, const std::string_view& yKey) const;

	const XmlNode* Child(const std::string_view& tag) const;
	std::vector<const XmlNode*> Children(const std::string_view& tag) const;
	bool HasChild(const std::string_view& tag) const;

	std::string m_Tag;
	std::string m_Content;
	std::unordered_map<std::string, std::string> m_Attributes;
	std::vector<XmlNode> m_Children;
};

#endif
