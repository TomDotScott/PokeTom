#ifndef TILERENDERER_H
#define TILERENDERER_H
#include <memory>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include "TileParser.h"
#include "../Engine/Rendering/SpriteBatcher.h"

struct TileRenderData
{
	uint32_t m_GlobalID;
	uint32_t m_LocalID;
	sf::Vector2f m_Position;
	std::shared_ptr<sf::Texture> m_Texture;
	sf::IntRect m_TextureRect;
	std::string m_LayerName;
	int m_ZIndex;
};

struct LayerBatcher
{
	std::string m_LayerName;
	int m_ZIndex;
	SpriteBatcher m_SpriteBatcher;
};

class TileRenderer
{
public:
	TileRenderer();
	void SetCameraCentre(const sf::Vector2f& position);
	void BuildBatches(const std::vector<TileRenderData>& tiles, const std::vector<TileLayerData>& layers);
	void Render(sf::RenderWindow& window) const;

private:
	std::vector<LayerBatcher> m_layerBatchers;

	sf::View m_cameraView;
};

#endif
