#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H
#include <filesystem>
#include "../Gameobject.h"
#include "../ISerialisable.h"

class UiElement : public GameObject, public ISerialisable
{
public:
	enum class eType : std::uint8_t
	{
		Button,
		Panel,
		Sprite,
		Text
	};

	enum class eLayer : std::int8_t
	{
		NONE = -1,
		BACKGROUND,
		MIDGROUND,
		FOREGROUND
	};

	UiElement(eType type);

	eLayer GetLayer() const;
	void SetLayer(eLayer layer);

	eType GetType() const;

	std::string GetName() const;

	virtual sf::Vector2f GetSize() const = 0;
	virtual void SetElementPosition(const sf::Vector2f& position);

	// TODO: It's getting to the point where a proper renderer class would be nice
	const std::vector<const sf::Drawable*>& GetDrawablesList() const;

	bool LoadFromXML(const XmlNode& node) override;

protected:
	void AddDrawable(const sf::Drawable* drawable);

private:
	std::string m_name;
	eLayer m_layer;
	eType m_type;
	std::vector<const sf::Drawable*> m_drawables;
};
#endif
