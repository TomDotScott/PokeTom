#ifndef STRINGTABLE_H
#define STRINGTABLE_H
#include <string>
#include <unordered_map>

#include "Factory.h"
#include "Language.h"

class StringTable : Factory<StringTable>
{
public:
	static StringTable* Get();
	friend class Factory;

	const std::string& GetString(const std::string& group, const std::string& stringID) const;

private:
	// KEY - group, VALUE - { KEY - ID, VALUE - String }
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_strings;

	StringTable();
	bool Init() override;
	bool Load(eLanguage language);
};

#define STRINGTABLE StringTable::Get()

#endif
