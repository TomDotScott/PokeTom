#include "WorldDefinition.h"

#include <algorithm>
#include <iostream>
#include <queue>

#include "Level.h"
#include "TileParser.h"
#include "../Engine/Entity.h"
#include "../Engine/EntityRegistry.h"
#include "../Engine/XML/XmlDocument.h"

constexpr static const char* OVERWORLD_ROOT_LEVEL = "starter_town";


WorldDefinition::WorldDefinition(sol::state& lua, const std::filesystem::path& worldDefinitionFilepath) :
	m_lua(&lua)
{
	XmlDocument xml;
	xml.Load(worldDefinitionFilepath);

	WorldDefinition::LoadFromXML(*xml.Root().Child(std::string{ "WorldDefinition" }));
}

std::optional<WorldDefinition::LevelTransition> WorldDefinition::EnterPortal(
	const std::string& levelName, const std::string& portalName) const
{
	const auto& currentLevelPortals = m_levelPortals.at(levelName);
	if (!currentLevelPortals.contains(portalName))
	{
		std::cerr << "WorldDefinition::OnPlayerEnterPortal - Portal with name " << portalName <<
			" Does not exist in level " << levelName << "\n";
		return std::nullopt;
	}

	const Portal& portalData = currentLevelPortals.at(portalName);

	if (!GetLevel(levelName)->OnDeactivate())
	{
		std::cerr << "WorldDefinition::EnterPortal - OnDeactivate failed to be called from the level's script!\n";
	}

	if (!GetLevel(portalData.m_TargetLevel)->OnActivate())
	{
		std::cerr << "WorldDefinition::EnterPortal - OnActivate failed to be called from the level's script!\n";
	}

	return LevelTransition{
		portalData.m_TargetLevel,
		portalData.m_TargetSpawnPoint
	};
}

// TODO: This should be per-level not all at once but hey-ho it's late and I am a hackerman
void WorldDefinition::LoadLevelScripts(sol::state& lua)
{
	for (const auto& level : m_levels | std::views::values)
	{
		// TODO: What should we do if a script fails?
		level->LoadLevelScript(lua);
	}
}

const WorldDefinition::Portal& WorldDefinition::GetPortalData(const std::string& levelName,
                                                              const std::string& portalName)
{
	return m_levelPortals.at(levelName).at(portalName);
}

std::vector<std::shared_ptr<Level>> WorldDefinition::GetLevelsIntersectingRect(const sf::FloatRect& rect) const
{
	std::vector<std::shared_ptr<Level>> intersectingLevels;

	for (const auto& level : m_levels | std::views::values)
	{
		const auto levelBounds = level->GetBounds();

		if (rect.findIntersection(levelBounds) != std::nullopt)
		{
			intersectingLevels.emplace_back(level);
		}
	}

	return intersectingLevels;
}

std::shared_ptr<Level> WorldDefinition::GetLevel(const std::string& name) const
{
	if (!m_levels.contains(name))
	{
		return nullptr;
	}

	return m_levels.at(name);
}

std::shared_ptr<Level> WorldDefinition::GetLevelAtPosition(const sf::Vector2f& position) const
{
	for (const auto& level : m_levels | std::views::values)
	{
		if (level->GetBounds().contains(position))
		{
			return level;
		}
	}

	return nullptr;
}

const Level::AdjacentLevels& WorldDefinition::GetAdjacentLevels(const std::string& levelName) const
{
	return GetLevel(levelName)->GetAdjacentLevels();
}

bool WorldDefinition::CanMoveTo(const Entity* entity, EntityRegistry& entities, const eDirection direction) const
{
	if (entity == nullptr)
	{
		return false;
	}

	const GridMovementComponent* entityMovement = entity->GetComponent<GridMovementComponent>();
	if (entityMovement == nullptr)
	{
		return false;
	}

	sf::Vector2f newPosition = entityMovement->GetNextPosition();

	newPosition = {
		std::round(newPosition.x / 32.f) * 32.f,
		std::round(newPosition.y / 32.f) * 32.f
	};

	// Check if the entity would be overlapping anyone
	if (entities.GetEntityAtPosition(newPosition) != ~0U)
	{
		return false;
	}

	const auto& currentLevel = GetLevelAtPosition(newPosition);
	return currentLevel != nullptr && currentLevel->CanMoveTo(newPosition);
}

void WorldDefinition::SortLevels()
{
	// If the provided starting level doesn't exist, pick the first parsed level as root
	std::string root = OVERWORLD_ROOT_LEVEL;
	if (!m_levels.contains(root))
	{
		root = m_levels.begin()->first;
		std::cerr << "WorldDefinition::ParseWorldDefinition - start level '" << OVERWORLD_ROOT_LEVEL <<
			"' not found. Using '" << root << "' as root.\n";
	}

	// BFS to determine the offsets of each of the levels (in tiles)
	std::unordered_map<std::string, sf::Vector2i> origins;
	std::queue<std::string> levelQueue;

	origins[root] = { 0, 0 };
	levelQueue.push(root);

	while (!levelQueue.empty())
	{
		const std::string current = levelQueue.front();
		levelQueue.pop();

		const auto& currentLevel = m_levels.at(current);
		const auto& [north, south, east, west] = currentLevel->GetAdjacentLevels();
		const sf::Vector2i& currentOrigin = origins.at(current);
		const int curCols = static_cast<int>(currentLevel->GetNumColumns());
		const int curRows = static_cast<int>(currentLevel->GetNumRows());

		auto tryAssignNeighbor = [&](const std::string& neighbourName,
		                             const std::function<sf::Vector2i()>& computeOrigin)
		{
			if (neighbourName.empty())
			{
				return;
			}

			if (!m_levels.contains(neighbourName))
			{
				return;
			}

			const sf::Vector2i proposedOrigin = computeOrigin();

			auto it = origins.find(neighbourName);
			if (it == origins.end())
			{
				origins[neighbourName] = proposedOrigin;
				levelQueue.push(neighbourName);
			}
			else
			{
				if (it->second != proposedOrigin)
				{
					std::cerr << "WorldDefinition::ParseWorldDefinition - placement conflict for level '" <<
						neighbourName << "'. Existing origin: ("
						<< it->second.x << "," << it->second.y << ") proposed: (" << proposedOrigin.x << "," <<
						proposedOrigin.y << ").\n";
					// Keep the existing placement (could add reconciliation logic here)
				}
			}
		};

		// North neighbour: it sits above current -> neighbour.origin.y = current.origin.y - neighbourRows
		tryAssignNeighbor(north, [&]() -> sf::Vector2i
		{
			const auto& neighbor = m_levels.at(north);
			return { currentOrigin.x, currentOrigin.y - static_cast<int>(neighbor->GetNumRows()) };
		});

		// South neighbour: it sits below current -> neighbour.origin.y = current.origin.y + currentRows
		tryAssignNeighbor(south, [&]() -> sf::Vector2i
		{
			return { currentOrigin.x, currentOrigin.y + curRows };
		});

		// East neighbour: sits to the right -> neighbour.origin.x = current.origin.x + currentCols
		tryAssignNeighbor(east, [&]() -> sf::Vector2i
		{
			return { currentOrigin.x + curCols, currentOrigin.y };
		});

		// West neighbour: sits to the left -> neighbour.origin.x = current.origin.x - neighbourCols
		tryAssignNeighbor(west, [&]() -> sf::Vector2i
		{
			const auto& neighbor = m_levels.at(west);
			return { currentOrigin.x - static_cast<int>(neighbor->GetNumColumns()), currentOrigin.y };
		});
	}

	// Apply the computed offsets to the levels. Some levels are not connected to the root (maybe offshoots, interiors, etc...)
	sf::Vector2i maxExtentStart(-0xFFFF, -0xFFFF);
	for (const auto& [levelName, origin] : origins)
	{
		const auto& levelPtr = m_levels.at(levelName);
		maxExtentStart.x = std::max<int>(origin.x * 32 + static_cast<int>(levelPtr->GetNumColumns()) * 32,
		                                 maxExtentStart.x);
		maxExtentStart.y = std::max<int>(origin.y * 32 + static_cast<int>(levelPtr->GetNumRows()) * 32,
		                                 maxExtentStart.y);
	}
	maxExtentStart.x += 3200;

	for (auto& [levelName, levelPtr] : m_levels)
	{
		if (const auto it = origins.find(levelName); it != origins.end())
		{
			levelPtr->SetWorldOrigin(it->second * 32);
		}
		else
		{
			levelPtr->SetWorldOrigin(maxExtentStart);
			maxExtentStart.y += static_cast<int>(levelPtr->GetNumRows()) * 32 * 2;
		}
	}
}

bool WorldDefinition::LoadFromXML(const XmlNode& node)
{
	const std::vector<const XmlNode*> levels = node.Children("Level");

	for (const auto& level : levels)
	{
		const std::string levelName = level->Attr("id", std::string{ "" });

		const std::filesystem::path tmjPath = level->Attr("tmj", std::string{ "" });
		if (tmjPath.empty() || !exists(tmjPath))
		{
			std::cerr << "WorldDefinition::ParseLevel - TMJ at path " << tmjPath <<
				" doesn't exist! Check the WorldDefinition.xml file\n";
			return false;
		}

		const auto tileMapData = TileParser::ParseTMJ(tmjPath);
		if (tileMapData == nullptr)
		{
			std::cerr << "WorldDefinition::ParseLevel - Failed to parse TMJ data from " << tmjPath <<
				"\n";
			return false;
		}

		Level::AdjacentLevels adjacentLevels{};
		if (const std::string northLevel = level->Attr("north", std::string{ "" }); !northLevel.empty())
		{
			adjacentLevels.m_North = northLevel;
		}
		if (const std::string southLevel = level->Attr("south", std::string{ "" }); !southLevel.empty())
		{
			adjacentLevels.m_North = southLevel;
		}
		if (const std::string eastLevel = level->Attr("east", std::string{ "" }); !eastLevel.empty())
		{
			adjacentLevels.m_North = eastLevel;
		}
		if (const std::string westLevel = level->Attr("west", std::string{ "" }); !westLevel.empty())
		{
			adjacentLevels.m_North = westLevel;
		}

		m_levels[levelName] = std::make_shared<Level>(
			*m_lua,
			levelName,
			tileMapData,
			adjacentLevels
		);

		const std::vector<const XmlNode*> portals = level->Children("Portal");

		for (const auto& portal : portals)
		{
			Portal p{
				.m_Name = portal->Attr("id", std::string{ "" }),
				.m_TargetLevel = portal->Attr("targetLevel", std::string{ "" }),
				.m_TargetSpawnPoint = portal->Attr("targetSpawnPoint", std::string{ "" })
			};

			if (p.m_Name.empty())
			{
				std::cerr << "WorldDefinition::ParsePortal - Failed to parse id for Portal in level " << levelName <<
					". Check the WorldDefinition XML and try again\n";
				return false;
			}

			if (p.m_TargetLevel.empty())
			{
				std::cerr << "WorldDefinition::ParsePortal - Failed to parse targetLevel for Portal in level " <<
					levelName <<
					". Check the WorldDefinition XML and try again\n";
				return false;
			}

			if (p.m_TargetSpawnPoint.empty())
			{
				std::cerr << "WorldDefinition::ParsePortal - Failed to parse targetSpawnPoint for Portal in level " <<
					levelName
					<< ". Check the WorldDefinition XML and try again\n";
				return false;
			}

			m_levelPortals[levelName][p.m_Name] = p;
		}
	}

	SortLevels();
	return true;
}
