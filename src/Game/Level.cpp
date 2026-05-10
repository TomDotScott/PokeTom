#include "Level.h"

#include "../Engine/Asserts.h"

Level::Level(sol::state& lua, const hash_type name, const std::shared_ptr<MapData>& mapData,
             const AdjacentLevels adjacentLevels) :
	m_name(name),
	m_mapData(mapData),
	m_adjacentLevels(adjacentLevels),
	m_hasLevelScript(false),
	m_levelScriptLoaded(false),
	m_entityZIndex(0),
	m_worldTileOrigin(0, 0),
	m_tileLogic(m_mapData)
{
	for (const auto& layer : m_mapData->m_Layers)
	{
		if (layer.m_IsPlayerLayer)
		{
			m_entityZIndex = layer.m_ZIndex;
			break;
		}
	}

	m_hasLevelScript = !m_mapData->m_LevelScript.empty();
}

bool Level::OnActivate()
{
	if (m_hasLevelScript)
	{
		if (!m_onActivate.valid())
		{
			return false;
		}

		auto x = m_onActivate(m_self);
		if (!x.valid())
		{
			sol::error err = x;
			ASSERT_MSG(false, "Level script OnActivate error: %s", err.what());
		}
	}

	m_active = true;
	return true;
}

bool Level::OnDeactivate()
{
	if (m_hasLevelScript)
	{
		if (!m_onDeactivate.valid())
		{
			return false;
		}

		auto x = m_onDeactivate(m_self);
		if (!x.valid())
		{
			sol::error err = x;
			ASSERT_MSG(false, "Level script OnDeactivate error: %s", err.what());
		}
	}

	m_active = false;
	return true;
}

bool Level::OnUpdate(float deltaTime)
{
	if (!m_active)
	{
		return true;
	}

	if (m_hasLevelScript)
	{
		if (!m_onUpdate.valid())
		{
			return false;
		}

		auto x = m_onUpdate(m_self, deltaTime);
		if (!x.valid())
		{
			sol::error err = x;
			ASSERT_MSG(false, "Level script update error: %s", err.what());
		}
	}

	return true;
}

const hash_type& Level::GetName() const
{
	return m_name;
}

std::vector<TileRenderData> Level::GetRenderData() const
{
	return m_tileLogic.BuildRenderData(m_worldTileOrigin);
}

std::vector<TileLayerData> Level::GetLayers() const
{
	return m_mapData->m_Layers;
}

uint32_t Level::GetNumColumns() const
{
	return m_mapData->m_NumColumns;
}

uint32_t Level::GetNumRows() const
{
	return m_mapData->m_NumRows;
}

uint32_t Level::GetTileWidth() const
{
	return m_mapData->m_TileWidth;
}

uint32_t Level::GetTileHeight() const
{
	return m_mapData->m_TileHeight;
}

int Level::GetEntityZIndex() const
{
	return m_entityZIndex;
}

bool Level::IsWorldSpacePointOnGrid(const sf::Vector2f& worldSpacePosition) const
{
	const sf::Vector2i gridPos = GetGridPositionFromWorldPosition(worldSpacePosition);

	return gridPos.x >= 0
		&& gridPos.x < static_cast<int>(GetNumColumns())
		&& gridPos.y >= 0
		&& gridPos.y < static_cast<int>(GetNumRows());
}

bool Level::CanMoveTo(const sf::Vector2f& worldSpacePosition) const
{
	if (!IsWorldSpacePointOnGrid(worldSpacePosition))
	{
		return false;
	}

	return GetTileAtPosition(worldSpacePosition, TileSheet::TileDefinition::IsBarrier) == nullptr;
}

const PortalTrigger* Level::GetPortalAtPosition(const sf::Vector2f worldSpacePosition) const
{
	const sf::Vector2i gridPosition = GetGridPositionFromWorldPosition(worldSpacePosition);
	for (const auto& portalData : m_mapData->m_Portals | std::views::values)
	{
		const bool xEqual = gridPosition.x == portalData.m_GridPosition.x || gridPosition.x == portalData.m_GridPosition
			.x + (portalData.m_Size.x - 1);

		const bool yEqual = gridPosition.y == portalData.m_GridPosition.y || gridPosition.y == portalData.m_GridPosition
			.y + (portalData.m_Size.y - 1);

		if (xEqual && yEqual)
		{
			return &portalData;
		}
	}

	return nullptr;
}

const SpawnPointData& Level::GetSpawnPointData(const hash_type& name) const
{
#if BUILD_DEBUG
	// If the map isn't found, assume we are debug-teleporting to a place
	if (!m_mapData->m_SpawnPoints.contains(name))
	{
		return m_mapData->m_SpawnPoints.begin()->second;
	}
#endif
	return m_mapData->m_SpawnPoints.at(name);
}

const Level::AdjacentLevels& Level::GetAdjacentLevels() const
{
	return m_adjacentLevels;
}

void Level::SetWorldOrigin(const sf::Vector2i& tileOrigin)
{
	m_worldTileOrigin = tileOrigin;
}

sf::FloatRect Level::GetBounds() const
{
	return {
		static_cast<sf::Vector2f>(m_worldTileOrigin),
		{
			static_cast<float>(GetNumColumns() * GetTileWidth()),
			static_cast<float>(GetNumRows() * GetTileHeight())
		}
	};
}

bool Level::LoadLevelScript(sol::state& lua)
{
	ASSERT(!m_levelScriptLoaded);

	// TODO: Remove this eventually, I think every level will have a script!
	if (!m_hasLevelScript)
	{
		return true;
	}

	m_self = lua.create_table();

	m_environment = sol::environment(lua, sol::create, lua.globals());

	const auto scriptPath = m_mapData->m_LevelScript.c_str();
	const auto result = lua.safe_script_file(m_mapData->m_LevelScript.generic_string(), m_environment);
	ASSERT_MSG(result.valid(), "Script %ls was not loaded correctly!", scriptPath);

	if (!result.valid())
	{
		return false;
	}

	sol::function init = m_environment["init"];
	if (init.valid())
	{
		init(m_self);
	}

	m_onUpdate = m_self["update"];
	ASSERT_MSG(m_onUpdate.valid(), "update function is missing from script %ls!", scriptPath);

	if (!m_onUpdate.valid())
	{
		return false;
	}

	m_onActivate = m_self["onActivate"];
	ASSERT_MSG(m_onActivate.valid(), "onActivate function is missing from script %ls!", scriptPath);

	if (!m_onActivate.valid())
	{
		return false;
	}

	m_onDeactivate = m_self["onDeactivate"];
	ASSERT_MSG(m_onDeactivate.valid(), "onDeactivate function is missing from script %ls!", scriptPath);

	if (!m_onDeactivate.valid())
	{
		return false;
	}

	return true;
}

const sf::Vector2i& Level::GetWorldOrigin() const
{
	return m_worldTileOrigin;
}

sf::Vector2i Level::GetGridPositionFromWorldPosition(const sf::Vector2f& worldSpacePosition) const
{
	const sf::Vector2i localPosition = static_cast<sf::Vector2i>(worldSpacePosition) - m_worldTileOrigin;
	const int gx = localPosition.x / static_cast<int>(GetTileWidth());
	const int gy = localPosition.y / static_cast<int>(GetTileHeight());
	return { gx, gy };
}

const TileSheet::TileDefinition* Level::GetTileAtPosition(const sf::Vector2f& worldSpacePosition, const unsigned tileFlags) const
{
	const sf::Vector2i gridPos = GetGridPositionFromWorldPosition(worldSpacePosition);
	const uint32_t index = gridPos.y * GetNumColumns() + gridPos.x;

	for (const TileLayerData& layer : GetLayers())
	{
		// Skip any layer that doesn't have this index
		if (index >= layer.m_LevelData.size())
		{
			continue;
		}

		const uint32_t globalTileID = layer.m_LevelData[index];
		if (globalTileID <= 0)
		{
			continue;
		}

		// Find which TileSheet this ID belongs to
		for (const auto& tileSheet : m_mapData->m_TileSheets | std::views::values)
		{
			if (globalTileID < tileSheet->GetFirstGID())
			{
				continue;
			}

			const uint32_t localID = globalTileID - tileSheet->GetFirstGID();

			const TileSheet::TileDefinition* tileDef = tileSheet->GetTileDefinition(localID);
			if (!tileDef)
			{
				continue;
			}

			// Check flag
			if (tileDef->m_Flags & tileFlags)
			{
				return tileDef;
			}
		}
	}

	return nullptr;
}
