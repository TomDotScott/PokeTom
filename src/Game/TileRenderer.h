#ifndef TILERENDERER_H
#define TILERENDERER_H
#include <memory>
#include <unordered_map>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include "../Engine/Rendering/SpriteBatcher.h"

struct TileRenderData
{
	uint32_t m_GlobalID;
	uint32_t m_LocalID;
	sf::Vector2f m_Position;
	std::shared_ptr<sf::Texture> m_Texture;
	sf::IntRect m_TextureRect;
	std::string m_LayerName;
};

class TileRenderer
{
public:
	void BuildBatches(const std::vector<TileRenderData>& tiles);
	void Render(sf::RenderWindow& window) const;

private:
	std::unordered_map<std::string, SpriteBatcher> m_layerBatchers;
};

#endif
