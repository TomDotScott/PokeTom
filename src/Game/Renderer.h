#ifndef TILERENDERER_H
#define TILERENDERER_H
#include <memory>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include "../Engine/Rendering/SpriteBatcher.h"
#include "TileParser.h"

class Entity;
class Player;

struct TileRenderData
{
	uint32_t m_GlobalID;
	uint32_t m_LocalID;
	sf::Vector2f m_Position;
	std::string m_SpriteSheetResourceName;
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

struct LevelRenderData
{
	std::vector<TileRenderData> m_TileRenderData;
	std::vector<TileLayerData> m_TileLayerData;
};

class Renderer
{
public:
	Renderer();

	// Sets the position of the camera view, clamped to the width and height of the map
	void SetCameraCentre(sf::Vector2f position, sf::FloatRect worldBounds);

	void BuildBatches(const std::unordered_map<std::string, LevelRenderData>& visibleLevelRenderData);
	void Render(sf::RenderWindow& window, const std::vector<std::shared_ptr<Entity>>& entities, int entityZIndex) const;

private:
	std::vector<LayerBatcher> m_layerBatchers;

	sf::View m_cameraView;
};

#endif
