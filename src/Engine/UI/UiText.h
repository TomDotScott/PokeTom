#ifndef UI_TEXT_H
#define UI_TEXT_H
#include <SFML/Graphics/Text.hpp>

#include "UiElement.h"

class UiText final : public UiElement
{
public:
	// TODO: Fix this up properly!
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
	}

	const char* GetText() const;

	void SetTextSize(unsigned size);

	void SetElementPosition(const sf::Vector2f& position) override;
	void RecalculatePositionAfterParentMoved() override;

	eAlignment GetAlignment() const;
	void SetAlignment(eAlignment alignment);

	sf::Vector2f GetSize() const override;

	bool LoadFromXML(const XmlNode& node) override;

private:
	std::string m_string;
	sf::Text m_text;
	eAlignment m_alignment;
};

#endif
