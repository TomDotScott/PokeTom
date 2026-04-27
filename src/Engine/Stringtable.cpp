#include "Stringtable.h"

#include "Asserts.h"
#include "CSV/CsvDocument.h"


StringTable* StringTable::Get()
{
	static std::shared_ptr<StringTable> st = nullptr;
	if (st == nullptr)
	{
		st = Create();
	}

	return st.get();
}

const std::string& StringTable::GetString(const std::string& group, const std::string& stringID) const
{
	ASSERT(m_strings.contains(group));
	ASSERT(m_strings.at(group).contains(stringID));

	return m_strings.at(group).at(stringID);
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
		const std::string groupName = row.m_Fields.at("group");
		std::string stringID = row.m_Fields.at("id");

		m_strings[groupName][stringID] = row.m_Fields.at(languageCode);
	}

	return true;
}

StringTable::StringTable() = default;
