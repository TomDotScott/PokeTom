#include "Stringtable.h"

#include <regex>
#include <ranges>
#include "Asserts.h"
#include "Language.h"
#include "Parsers/CSV/CsvDocument.h"


StringTable* StringTable::Get()
{
	static std::shared_ptr<StringTable> st = nullptr;
	if (st == nullptr)
	{
		st = Create();
	}

	return st.get();
}

bool StringTable::Exists(const hash_type& group, const hash_type& stringID) const
{
	if (!m_strings.contains(group))
	{
		return false;
	}

	return m_strings.at(group).contains(stringID);
}

void StringTable::AddCustomString(const hash_type& group, const hash_type& stringID, const std::string& value)
{
	ASSERT(m_strings.contains(group));
#if BUILD_DEBUG
	ASSERT_MSG(!m_strings.at(group).contains(stringID), "String %s already exists in the stringtable group %s!", stringID.c_str(), group.c_str());
#else
	ASSERT_MSG(!m_strings.at(group).contains(stringID), "String with hash %u already exists in the stringtable!", stringID);
#endif

	m_strings[group][stringID] = value;
}

std::string StringTable::GetString(const hash_type& stringID) const
{
	// If the group is empty, find the first string that matches the ID
	for (const auto& groupContents : m_strings | std::views::values)
	{
		if (groupContents.contains(stringID))
		{
			return groupContents.at(stringID);
		}
	}

#if BUILD_DEBUG
	ASSERT_MSG(false, "String with ID: %s is not present anywhere in the stringtable!", stringID.c_str());
	return "@NO_STRING@_" + stringID;
#else
	ASSERT(false);
	return "@NO_STRING@_" + std::to_string(stringID);
#endif
}

std::string StringTable::GetString(const hash_type& group, const hash_type& stringID) const
{
#if BUILD_DEBUG
	if (!Exists(group, stringID))
	{
		ASSERT(false);
		return "!!!UNKNOWN_STRING!!! " + stringID;
	}
#endif

	ASSERT(Exists(group, stringID));

	const std::string& s = m_strings.at(group).at(stringID);

	std::string result;
	result.reserve(s.size());

	// Parse the string for {STRING_IDs} and replace them with the relevant string entry
	std::regex re("\\{([^}]+)\\}");

	auto begin = std::sregex_iterator(s.begin(), s.end(), re);
	auto end = std::sregex_iterator();

	std::size_t lastPos = 0;

	for (auto i = begin; i != end; ++i)
	{
		const std::smatch& match = *i;

		result.append(s, lastPos, match.position() - lastPos);

		std::string captured = match[1].str();

		result += GetString(HASH(captured));

		lastPos = match.position() + match.length();
	}

	// Append remainder
	result.append(s, lastPos, std::string::npos);

	return result;
}

bool StringTable::Init()
{
	return Load(CHOSEN_LANGUAGE);
}

bool StringTable::Load(const eLanguage language)
{
	CsvDocument doc;
	doc.Load("strings.csv");

	const std::string languageCode = GetLanguageCode(language);

	for (const auto& row : doc.Rows())
	{
		hash_type groupName = HASH(row.m_Fields.at("group"));
		hash_type stringID = HASH(row.m_Fields.at("id"));

		m_strings[groupName][stringID] = row.m_Fields.at(languageCode);
	}

	return true;
}

StringTable::StringTable() = default;
