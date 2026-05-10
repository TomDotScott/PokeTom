#ifndef STRINGTABLE_H
#define STRINGTABLE_H
#include <string>
#include <unordered_map>

#include "Factory.h"
#include "Hash.h"
#include "Language.h"

class StringTable : Factory<StringTable>
{
public:
	static StringTable* Get();
	friend class Factory;

	bool Exists(const hash_type& group, const hash_type& stringID) const;

	void AddCustomString(const hash_type& group, const hash_type& stringID, const std::string& value);
	std::string GetString(const hash_type& stringID) const;
	std::string GetString(const hash_type& group, const hash_type& stringID) const;

private:
	// KEY - group, VALUE - { KEY - ID, VALUE - String }
	std::unordered_map<hash_type, std::unordered_map<hash_type, std::string>> m_strings;

	StringTable();
	bool Init() override;
	bool Load(eLanguage language);
};

#define STRINGTABLE StringTable::Get()

#endif
