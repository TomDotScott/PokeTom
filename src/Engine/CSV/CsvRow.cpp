#include "CsvRow.h"

#include <algorithm>


std::string CsvRow::Col(const std::string_view& header, const std::string& fallback)
{
	const auto it = m_Fields.find(std::string{ header });
	return it == m_Fields.end() ? fallback : it->second;
}

int CsvRow::Col(const std::string_view& header, const int fallback)
{
	const auto it = m_Fields.find(std::string{ header });
	if (it == m_Fields.end()) return fallback;
	try { return std::stoi(it->second); }
	catch (...) { return fallback; }
}

unsigned CsvRow::Col(const std::string_view& header, const unsigned fallback)
{
	const auto it = m_Fields.find(std::string{ header });
	if (it == m_Fields.end()) return fallback;
	try { return std::stoul(it->second); }
	catch (...) { return fallback; }
}

float CsvRow::Col(const std::string_view& header, const float fallback)
{
	const auto it = m_Fields.find(std::string{ header });
	if (it == m_Fields.end()) return fallback;
	try { return std::stof(it->second); }
	catch (...) { return fallback; }
}

bool CsvRow::Col(const std::string_view& header, const bool fallback)
{
	const auto it = m_Fields.find(std::string{ header });
	if (it == m_Fields.end()) return fallback;

	std::string value = it->second;
	std::ranges::transform(value, value.begin(), [](const unsigned char c) { return std::tolower(c); });

	if (value == "true") return true;
	if (value == "false") return false;
	return fallback;
}

bool CsvRow::Has(const std::string_view& header) const
{
	return m_Fields.contains(std::string{ header });
}
