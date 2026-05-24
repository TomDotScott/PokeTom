#ifndef UI_PANEL_H
#define UI_PANEL_H

#include "UiElement.h"
#include "../Asserts.h"

class UiPanel : public UiElement
{
public:
	UiPanel(UiElement* parent = nullptr);

	void SetElementPosition(const sf::Vector2f& position) override;
	void RecalculatePositionAfterParentMoved() override;

	sf::Vector2f GetSize() const override;
	void SetSize(const sf::Vector2f& size);

	UiElement* GetChild(const std::string& name) const;
	const std::vector<std::unique_ptr<UiElement>>& GetChildren() const;

	void OnActivate() override;

private:
	std::vector<std::unique_ptr<UiElement>> m_children;

	sf::Vector2f m_size;

	bool LoadFromXML(const XmlNode& node) override;

	template <typename T>
	bool LoadChildrenOfType(const XmlNode& node, const std::string_view& tag)
	{
		return std::ranges::all_of(node.Children(tag), [&](const auto& c)
		{
			if (c == nullptr)
			{
				return true;
			}

			T* child = dynamic_cast<T*>(m_children.emplace_back(std::make_unique<T>(this)).get());
			ASSERT_MSG(child != nullptr, "SOMETHING HAS GONE VERY WRONG IF THIS ASSERT IS HIT!");

			if (!child->LoadFromXML(*c))
			{
				ASSERT_MSG(false, "UiPanel: Error loading panel %s", GetName().c_str());
				return false;
			}

			return true;
		});
	}
};

#endif
