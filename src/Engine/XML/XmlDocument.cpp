#include "XmlDocument.h"

#include <fstream>
#define HOXML_IMPLEMENTATION
#include <hoxml.h>
#include <iostream>
#include <stack>

namespace
{
	void BuildNode(hoxml_context_t& context, const char* xml, const size_t xmlLength, XmlNode& root)
	{
		hoxml_code_t code;

		std::stack<XmlNode*> stack;
		stack.push(&root);

		while ((code = hoxml_parse(&context, xml, xmlLength)) != HOXML_END_OF_DOCUMENT)
		{
			if (code == HOXML_ERROR_TAG_MISMATCH)
			{
				std::cerr << "XmlDocument - Tag mismatch on line " << context.line << "\n";
				return;
			}

			if (code == HOXML_ELEMENT_BEGIN)
			{
				// Give a child to the parent at the top of the stack
				XmlNode& parent = *stack.top();
				parent.m_Children.emplace_back();

				XmlNode* newNode = &parent.m_Children.back();
				newNode->m_Tag = context.tag;

				stack.push(newNode);
			}
			else if (code == HOXML_ATTRIBUTE)
			{
				stack.top()->m_Attributes[context.attribute] = context.value;
			}
			else if (code == HOXML_ELEMENT_END)
			{
				if (context.content)
				{
					stack.top()->m_Content = context.content;
				}

				stack.pop();
			}
		}
	}
}

bool XmlDocument::Load(const std::filesystem::path& path)
{
	if (!std::filesystem::exists(path))
	{
		std::cerr << "XmlDocument::Load - XML file with path " << path << " does not exist!\n";
		return false;
	}

	std::ifstream file(path);
	if (!file)
	{
		std::cerr << "XmlDocument::Load - Unable to open XML file with path " << path << "!\n";
		return false;
	}

	std::ostringstream ss;
	ss << file.rdbuf();

	const std::string text = ss.str();
	if (text.empty())
	{
		std::cerr << "XmlDocument::Load - " << path << " is an empty file!\n";
		return false;
	}

	const char* xml = text.c_str();
	const size_t xmlLength = text.size();

	std::vector<char> buffer(xmlLength * 2);
	hoxml_context_t context;

	hoxml_init(&context, buffer.data(), buffer.size());

	m_root.m_Tag = "root";
	BuildNode(context, xml, xmlLength, m_root);
	return true;
}

const XmlNode& XmlDocument::Root() const
{
	return m_root;
}
