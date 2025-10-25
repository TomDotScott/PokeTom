#ifndef SPRITEBATCHER_H
#define SPRITEBATCHER_H

#include <vector>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <memory>

class SpriteBatcher final : public sf::Drawable
{
public:
	explicit SpriteBatcher(const std::shared_ptr<sf::Texture>& masterTexture);
	void BatchSprites(const std::vector<sf::Sprite>& sprites);

private:
	std::shared_ptr<sf::Texture> m_masterTexture;
	std::vector<sf::Vertex> m_vertices{};

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void SetQuad(const sf::Sprite* sprite, std::size_t startVertex);
};

#endif
