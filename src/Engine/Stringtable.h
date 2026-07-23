#ifndef STRINGTABLE_H
#define STRINGTABLE_H
#include <regex>
#include <string>
#include <unordered_map>

#include "Asserts.h"
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

	template <typename... Args>
	std::string GetDynamicString(const hash_type& stringID, Args... fmt)
	{
		std::string baseString = GetString(HASH("DYNAMIC_STRING"), stringID);

		std::string result;
		result.reserve(baseString.size() * 2);

		// Parse the string for %sN and replace with the variadic arg
		std::regex re(R"(%\d+)");

		auto begin = std::sregex_iterator(baseString.begin(), baseString.end(), re);
		auto end = std::sregex_iterator();

		auto params = std::array{ fmt... };
		std::size_t lastPos = 0;

		for (auto i = begin; i != end; ++i)
		{
			const std::smatch& match = *i;

			result.append(baseString, lastPos, match.position() - lastPos);

			const std::string captured = match[0].str();

			int idx = std::stoi(captured.substr(1));

			result += params[idx - 1];

			lastPos = match.position() + match.length();
		}

		// Append remainder
		result.append(baseString, lastPos, std::string::npos);

		return result;
	}

private:
	// KEY - group, VALUE - { KEY - ID, VALUE - String }
	std::unordered_map<hash_type, std::unordered_map<hash_type, std::string>> m_strings;

	StringTable();
	bool Init() override;
	bool Load(eLanguage language);
};

#define STRINGTABLE StringTable::Get()

#endif
