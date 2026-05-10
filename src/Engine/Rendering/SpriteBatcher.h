#ifndef SPRITEBATCHER_H
#define SPRITEBATCHER_H

#include <vector>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <memory>
#include <string>
#include "../Hash.h"

class SpriteBatcher final : public sf::Drawable
{
public:
	SpriteBatcher() = default;
	SpriteBatcher(hash_type masterTextureResourceName);
	void BatchSprites(const std::vector<sf::Sprite>& sprites);

private:
	hash_type m_masterTextureResourceName;
	std::vector<sf::Vertex> m_vertices{};

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void SetQuad(const sf::Sprite* sprite, std::size_t startVertex);
};

#endif
