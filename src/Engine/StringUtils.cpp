#include "StringUtils.h"

#include <sstream>

string_utils::text_pages string_utils::WrapToPages(const std::string& text, unsigned charsPerLine, unsigned maxLines)
{
	// Split the string into words so that we don't split m
	// idword (lol)
	std::vector<std::string> words;
	std::istringstream stream(text);
	std::string word;
	while (stream >> word)
	{
		words.emplace_back(std::move(word));
	}

	std::vector<std::string> pages;
	std::string currentLine;
	std::vector<std::string> currentPageLines;

	auto flush = [&]()
	{
		if (!currentLine.empty())
		{
			currentPageLines.emplace_back(std::move(currentLine));
		}

		std::string page;
		for (size_t i = 0; i < currentPageLines.size(); ++i)
		{
			if (i > 0)
			{
				page += "\n";
			}

			page += currentPageLines[i];
		}

		pages.emplace_back(std::move(page));
		currentPageLines.clear();
		currentLine.clear();
	};

	for (const auto& w : words)
	{
		const size_t len = currentLine.empty() ? w.size() : currentLine.size() + 1 + w.size();

		if (!currentLine.empty() && len > charsPerLine)
		{
			currentPageLines.emplace_back(std::move(currentLine));
			currentLine.clear();

			if (currentPageLines.size() >= maxLines)
			{
				flush();
			}
		}

		if (!currentLine.empty())
			currentLine += ' ';

		currentLine += w;
	}

	if (!currentLine.empty() || !currentPageLines.empty())
	{
		flush();
	}

	return pages;
}
