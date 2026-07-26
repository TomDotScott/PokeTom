#ifndef UIPROGRESSBAR_H
#define UIPROGRESSBAR_H

#include <SFML/Graphics/RectangleShape.hpp>

#include "UiElement.h"

class UiProgressBar : public UiElement
{
public:
	enum class eFillOrientation
	{
		Horizontal,
		Vertical
	};

	UiProgressBar(UiElement* parent = nullptr);
	sf::Vector2f GetSize() const override;
	void RecalculatePositionAfterParentMoved() override;

	void OnActivate() override;
	void OnDeactivate() override;

	void SetElementPosition(const sf::Vector2f& position) override;
	bool LoadFromXML(const XmlNode& node) override;

	// 0.0 - 1.0
	void SetProgress(float progress);

	void SetProgress(float val, float maxVal, bool invert);

private:
	sf::RectangleShape m_fill;
	float m_maxFillSize;

	sf::RectangleShape m_background;
	eFillOrientation m_fillOrientation;

	float m_progress;

	void UpdateFill();
};


#endif // UIPROGRESSBAR_H
