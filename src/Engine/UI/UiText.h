#ifndef UI_TEXT_H
#define UI_TEXT_H
#include <SFML/Graphics/Text.hpp>

#include "UiElement.h"
#include "../Globals.h"

class UiText final : public UiElement
{
public:
	enum class eAlignment : uint8_t
	{
		Left = 0,
		Centre,
		Right
	};

	UiText(UiElement* parent = nullptr);

	template<typename... Args>
	void SetText(const char* fmt, Args... args)
	{
		char buffer[512]{ 0 };

		if (sprintf_s(buffer, fmt, args...) == -1)
		{
			printf(" UiText: Error writing text arguments\n");
			return;
		}

		m_string = &buffer[0];
		m_text.setString(m_string);

		// TODO: Right alignment
		if (m_alignment == eAlignment::Centre)
		{
			CalculateAlignmentBounds();
			m_text.setPosition(m_position);
		}
	}

	const char* GetText() const;

	void SetTextSize(unsigned size);

	void SetPosition(const sf::Vector2f& position) override;

	eAlignment GetAlignment() const;
	void SetAlignment(eAlignment alignment);

	sf::Vector2f GetSize() const override;

private:
	std::string m_string;
	sf::Text m_text;
	eAlignment m_alignment;
	UiElement* m_parent;

	void CalculateAlignmentBounds();

	bool ParseEndElement(hoxml_context_t*& context) override;
};

// TODO: This could probably be an extension of the UiText class!
struct OffsetUiText
{
	UiText* m_text = nullptr;
	sf::Vector2f m_offset = VECTOR2F_ZERO;
};
#endif
