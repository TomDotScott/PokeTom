#ifndef STRINGUTILS_H
#define STRINGUTILS_H
#include <string>
#include <vector>

namespace string_utils {
	typedef std::vector<std::string> text_pages;
	text_pages WrapToPages(const std::string& text, unsigned charsPerLine = 64, unsigned maxLines = 3);
};

#endif // STRINGUTILS_H
