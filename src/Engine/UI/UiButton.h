#ifndef UI_BUTTON_H
#define UI_BUTTON_H
#include "UiElement.h"
#include "UiSprite.h"
#include "UiText.h"
#include "../Event.h"

class UiButton : public UiElement
{
public:
	UiButton(UiElement* parent = nullptr);

	void SetElementPosition(const sf::Vector2f& position) override;
	void RecalculatePositionAfterParentMoved() override;

	void OnButtonPressed(const std::function<void()>& callback);

	// TODO: I don't like this being public. I think it should be private with the UIManager as a friend class
	void OnLeftClickPressed();

	sf::Vector2f GetBottomRight() const;

	sf::Vector2f GetSize() const override;

	UiText* GetText() const;

	bool LoadFromXML(const XmlNode& node) override;

private:
	// Required!
	std::unique_ptr<UiSprite> m_sprite;

	// Optional, the m_text pointer may be null
	std::unique_ptr<UiText> m_text;

	Event<> m_pressedEvent;
};

#endif