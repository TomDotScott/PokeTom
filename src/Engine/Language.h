#ifndef LANGUAGE_H
#define LANGUAGE_H

enum class eLanguage
{
	English,
	French,
	Italian,
	German,
	Spanish
};

inline const char* GetLanguageCode(const eLanguage language)
{
	switch (language) {
	case eLanguage::English:
		return "en";
	case eLanguage::French:
		return "fr";
	case eLanguage::Italian:
		return "it";
	case eLanguage::German:
		return "de";
	case eLanguage::Spanish:
		return "es";
	}

	return "en";
}

extern eLanguage CHOSEN_LANGUAGE;

#endif