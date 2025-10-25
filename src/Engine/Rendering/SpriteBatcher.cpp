#include "SpriteBatcher.h"

SpriteBatcher::SpriteBatcher(const std::shared_ptr<sf::Texture>& masterTexture) :
	m_masterTexture(masterTexture)
{
}

void SpriteBatcher::BatchSprites(const std::vector<sf::Sprite>& sprites)
{
	m_vertices.resize(sprites.size() * 6u);

	for (std::size_t i{ 0u }; i < sprites.size(); ++i)
	{
		SetQuad(&(sprites[i]), i * 6u);
	}
}

void SpriteBatcher::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.texture = m_masterTexture.get();
	target.draw(m_vertices.data(),
		m_vertices.size(),
		sf::PrimitiveType::Triangles,
		states
	);
}

void SpriteBatcher::SetQuad(const sf::Sprite* sprite, std::size_t startVertex)
{
	const sf::Transform transform{ sprite->getTransform() };
	const sf::Color color{ sprite->getColor() };
	const sf::IntRect rect{ sprite->getTextureRect() };

	sf::Vector2f shapeTopLeft{ 0.f, 0.f };
	sf::Vector2f shapeBottomRight(rect.size);
	sf::Vector2f shapeTopRight{ shapeBottomRight.x, shapeTopLeft.y };
	sf::Vector2f shapeBottomLeft{ shapeTopLeft.x, shapeBottomRight.y };

	const sf::Vector2f textureTopLeft(rect.position);
	const sf::Vector2f textureBottomRight{ textureTopLeft + shapeBottomRight };
	const sf::Vector2f textureTopRight{ textureBottomRight.x, textureTopLeft.y };
	const sf::Vector2f textureBottomLeft{ textureTopLeft.x, textureBottomRight.y };

	shapeTopLeft = transform.transformPoint(shapeTopLeft);
	shapeBottomRight = transform.transformPoint(shapeBottomRight);
	shapeTopRight = transform.transformPoint(shapeTopRight);
	shapeBottomLeft = transform.transformPoint(shapeBottomLeft);

	m_vertices[startVertex + 0u].position = shapeTopLeft;
	m_vertices[startVertex + 0u].texCoords = textureTopLeft;
	m_vertices[startVertex + 0u].color = color;

	m_vertices[startVertex + 1u].position = shapeBottomLeft;
	m_vertices[startVertex + 1u].texCoords = textureBottomLeft;
	m_vertices[startVertex + 1u].color = color;

	m_vertices[startVertex + 2u].position = shapeBottomRight;
	m_vertices[startVertex + 2u].texCoords = textureBottomRight;
	m_vertices[startVertex + 2u].color = color;

	m_vertices[startVertex + 5u].position = shapeTopRight;
	m_vertices[startVertex + 5u].texCoords = textureTopRight;
	m_vertices[startVertex + 5u].color = color;

	m_vertices[startVertex + 3u] = m_vertices[startVertex + 0u];
	m_vertices[startVertex + 4u] = m_vertices[startVertex + 2u];
}
