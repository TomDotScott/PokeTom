#ifndef XMLDOCUMENT_H
#define XMLDOCUMENT_H
#include <filesystem>

#include "XmlNode.h"

class XmlDocument
{
public:
	bool Load(const std::filesystem::path& path);
	const XmlNode& Root() const;

private:
	XmlNode m_root;
};

#endif
