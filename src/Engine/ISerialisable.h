#ifndef ISERIALISABLE_H
#define ISERIALISABLE_H

#include "XML/XmlNode.h"

class ISerialisable
{
public:
	virtual bool LoadFromXML(const XmlNode& node) = 0;
	virtual ~ISerialisable() = default;
};

#endif
