#include "CsvDocument.h"

#include <fstream>
#include <iostream>

#include "../Asserts.h"

bool CsvDocument::Load(const std::filesystem::path& path, const char delimiter)
{
	if (!std::filesystem::exists(path))
	{
		std::cerr << "CsvDocument::Load - " << path << " does not exist!\n";
		return false;
	}

	std::ifstream file;
	file.open(path);
	if (!file)
	{
		std::cerr << "CsvDocument::Load - Unable to load " << path << " \n";
		return false;
	}

	std::string line;
	if (!std::getline(file, line) || line.empty())
	{
		std::cerr << "CsvDocument::Load - CSV " << path << " has no header row!\n";
		return false;
	}

	if (!line.empty() && line.back() == '\r')
	{
		line.pop_back();
	}

	std::vector<std::string> headers = ParseLine(line, delimiter);
	std::vector<CsvRow> rows;

	while (std::getline(file, line))
	{
		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}

		if (line.empty())
		{
			continue;
		}

		const auto fields = ParseLine(line, delimiter);

		CsvRow row;
		for (size_t i = 0; i < headers.size(); ++i)
		{
			ASSERT(i < fields.size());

			row.m_Fields[headers[i]] = fields[i];
		}

		rows.emplace_back(row);
	}


	m_headers = std::move(headers);
	m_rows = std::move(rows);
	return true;
}

const std::vector<CsvRow>& CsvDocument::Rows() const
{
	return m_rows;
}

const std::vector<std::string>& CsvDocument::Headers() const
{
	return m_headers;
}

std::vector<std::string> CsvDocument::ParseLine(const std::string& line, char delimiter)
{
	std::vector<std::string> fields;
	std::string field;
	bool inQuotes = false;

	for (size_t i = 0; i < line.size(); ++i)
	{
		const char c = line[i];
		if (inQuotes)
		{
			if (c == '"')
			{
				// Are they escaped?
				if (i + 1 < line.size() && line[i + 1] == '"')
				{
					field += '"';
					++i;
				}
				else
				{
					inQuotes = false;
				}
			}
			else
			{
				field += c;
			}
		}
		else
		{
			if (c == '"')
			{
				inQuotes = true;
			}
			else if (c == delimiter)
			{
				fields.emplace_back(std::move(field));
				field.clear();
			}
			else
			{
				field += c;
			}
		}
	}

	fields.emplace_back(std::move(field));
	return fields;
}
