#ifndef LEVEL_H
#define LEVEL_H
#include <sol/sol.hpp>

#include "TileLogic.h"
#include "Renderer.h"

class Level
{
public:
	struct AdjacentLevels
	{
		hash_type m_North;
		hash_type m_South;
		hash_type m_East;
		hash_type m_West;
	};

	explicit Level(sol::state& lua, hash_type name, const std::shared_ptr<MapData>& mapData,
	               AdjacentLevels adjacentLevels);

	// Update Level Lua script
	bool OnUpdate(float deltaTime);

	// Setup or remove Lua scripts and entities
	bool OnActivate();
	bool OnDeactivate();

	const hash_type& GetName() const;

	const std::vector<TileRenderData>& GetRenderData() const;
	std::vector<TileLayerData> GetLayers() const;

	uint32_t GetNumColumns() const;
	uint32_t GetNumRows() const;

	uint32_t GetTileWidth() const;
	uint32_t GetTileHeight() const;

	int GetEntityZIndex() const;

	bool IsWorldSpacePointOnGrid(const sf::Vector2f& worldSpacePosition) const;
	bool CanMoveTo(const sf::Vector2f& worldSpacePosition) const;

	const PortalTrigger* GetPortalAtPosition(sf::Vector2f worldSpacePosition) const;

	sf::Vector2i GetGridPositionFromWorldPosition(const sf::Vector2f& worldSpacePosition) const;

	const TileSheet::TileDefinition* GetTileAtPosition(const sf::Vector2f& worldSpacePosition, unsigned tileFlags = TileSheet::TileDefinition::None) const;

	const SpawnPointData& GetSpawnPointData(const hash_type& name) const;

	const AdjacentLevels& GetAdjacentLevels() const;

	// The level's top-leftmost tile coordinates in world-space
	const sf::Vector2i& GetWorldOrigin() const;

	// Called after WorldDefinition computes the offset positions
	void SetWorldOrigin(const sf::Vector2i& tileOrigin);

	sf::FloatRect GetBounds() const;

	bool LoadLevelScript(sol::state& lua);

private:
	hash_type m_name;
	std::shared_ptr<MapData> m_mapData;
	AdjacentLevels m_adjacentLevels;

	bool m_hasLevelScript;
	bool m_levelScriptLoaded;
	bool m_active;
	sol::table m_self;
	sol::environment m_environment;
	sol::function m_onUpdate;
	sol::function m_onActivate;
	sol::function m_onDeactivate;

	mutable bool m_renderDataIsDirty;
	mutable std::vector<TileRenderData> m_cachedRenderData;

	int m_entityZIndex;

	// The position of the top-left tile in world-space coordinates
	sf::Vector2i m_worldTileOrigin;

	TileLogic m_tileLogic;
};

#endif // LEVEL_H
