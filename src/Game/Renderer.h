#ifndef TILERENDERER_H
#define TILERENDERER_H
#include <memory>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
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
	std::optional<std::vector<TileSheet::TileDefinition::AnimFrame>> m_AnimatedFrames = std::nullopt;
};

struct AnimatedTileInstance
{
	hash_type m_LayerKey;
	int m_ZIndex;
	hash_type m_TextureHash;
	sf::Vector2f m_Position;
	uint32_t m_TypeID;
};

struct AnimatedTileType
{
	const sf::Texture* m_Texture;
	sf::Vector2i m_TileSize;
	uint32_t m_TextureTilesPerRow;
	std::vector<TileSheet::TileDefinition::AnimFrame> m_Frames;

	// A SHARED clock so all of the tiles animate at the same time
	uint32_t m_CurrentFrame = 0;
	float m_ElapsedMS = 0;

	sf::VertexArray m_Vertices;
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

	void UpdateAnimatedTiles(float deltaTime);

	void BuildBatches(const std::unordered_map<hash_type, LevelRenderData>& visibleLevelRenderData);
	void RenderLevel(sf::RenderWindow& window, const EntityRegistry& entities, int entityZIndex);

	void RenderEntity(sf::RenderWindow& window, const Entity* entity) const;

private:
	std::vector<LayerBatcher> m_layerBatchers;

	std::vector<AnimatedTileType> m_animatedTileTypes;
	std::unordered_map<hash_type, size_t> m_animTypeIndices;

	float m_zoom;
	sf::View m_cameraView;

	void RenderEntities(sf::RenderWindow& window, const EntityRegistry& entities) const;

	void PatchUVs(AnimatedTileType& type);

	void OnScreenResized(sf::Vector2u newSize);
};

#endif
