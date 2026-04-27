#ifndef CSVROW_H
#define CSVROW_H
#include <string>
#include <unordered_map>

struct CsvRow
{
	std::string Col(const std::string_view& header, const std::string& fallback = "");
	int Col(const std::string_view& header, int fallback = 0);
	unsigned Col(const std::string_view& header, unsigned fallback = 0U);
	float Col(const std::string_view& header, float fallback = 0.f);
	bool Col(const std::string_view& header, bool fallback = false);

	bool Has(const std::string_view& header) const;

	std::unordered_map<std::string, std::string> m_Fields;
};

#endif
