#ifndef CSVDOCUMENT_H
#define CSVDOCUMENT_H
#include <filesystem>

#include "CsvRow.h"

class CsvDocument
{
public:
	bool Load(const std::filesystem::path& path, char delimiter = ',');

	const std::vector<CsvRow>& Rows() const;
	const std::vector<std::string>& Headers() const;

private:
	std::vector<std::string> m_headers;
	std::vector<CsvRow> m_rows;

	static std::vector<std::string> ParseLine(const std::string& line, char delimiter);
};


#endif
