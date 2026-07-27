#ifndef TILERENDERER_H
#define TILERENDERER_H
#include <memory>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include "../Engine/Rendering/SpriteBatcher.h"
#include "TileParser.h"

class EntityRegistry;
class Entity;
class Player;

struct TileRenderData
{
	uint32_t m_GlobalID;
	uint32_t m_LocalID;
	sf::Vector2f m_Position;
	hash_type m_SpriteSheetResourceName;
	sf::IntRect m_TextureRect;
	hash_type m_LayerName;
	int m_ZIndex;
};

struct LayerBatcher
{
	hash_type m_LayerName;
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

	void SetZoom(float zoom);
	float GetZoom() const;

	void BuildBatches(const std::unordered_map<hash_type, LevelRenderData>& visibleLevelRenderData);
	void RenderLevel(sf::RenderWindow& window, const EntityRegistry& entities, int entityZIndex);

	void RenderEntity(sf::RenderWindow& window, const Entity* entity) const;

private:
	std::vector<LayerBatcher> m_layerBatchers;
	float m_zoom;
	sf::View m_cameraView;

	void RenderEntities(sf::RenderWindow& window, const EntityRegistry& entities) const;
	void OnScreenResized(sf::Vector2u newSize);
};

#endif
