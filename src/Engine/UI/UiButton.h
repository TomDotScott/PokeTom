#ifndef UI_BUTTON_H
#define UI_BUTTON_H
#include "UiElement.h"
#include "UiSprite.h"
#include "UiText.h"
#include "../Event.h"

class UiButton : public UiElement
{
public:
	UiButton();

	void SetPosition(const sf::Vector2f& position) override;

	void OnButtonPressed(const std::function<void()>& callback);

	// TODO: I don't like this being public. I think it should be private with the UIManager as a friend class
	void OnLeftClickPressed();

	sf::Vector2f GetBottomRight() const;

	sf::Vector2f GetSize() const override;

	UiText* GetText() const;

private:
	// Required!
	UiSprite* m_sprite;

	// Optional, the m_text pointer may be null
	OffsetUiText m_offsetText;

	Event<> m_pressedEvent;

	bool ParseBeginElement(hoxml_context_t*& context) override;
	bool ParseEndElement(hoxml_context_t*& context) override;
};

#endif