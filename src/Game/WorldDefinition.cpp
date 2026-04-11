#include "WorldDefinition.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>

#include "Level.h"
#include "TileParser.h"
#include "../Engine/Entity.h"

constexpr static const char* OVERWORLD_ROOT_LEVEL = "starter_town";


WorldDefinition::WorldDefinition(sol::state& lua, const std::filesystem::path& worldDefinitionFilepath)
{
	ParseWorldDefinition(lua, worldDefinitionFilepath);
}

std::optional<WorldDefinition::LevelTransition> WorldDefinition::EnterPortal(const std::string& levelName, const std::string& portalName) const
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

bool WorldDefinition::CanMoveTo(const Entity* entity, const eDirection direction) const
{
	const GridMovementComponent* entityMovement = entity->GetComponent<GridMovementComponent>();
	if (entityMovement == nullptr)
	{
		return false;
	}

	sf::Vector2f moveDirection(0, 0);

	switch (direction)
	{
	case eDirection::North:
		moveDirection.y -= 1;
		break;
	case eDirection::South:
		moveDirection.y += 1;
		break;
	case eDirection::West:
		moveDirection.x -= 1;
		break;
	case eDirection::East:
		moveDirection.x += 1;
		break;
	case eDirection::None:
		break;
	}

	sf::Vector2f newPosition = entityMovement->GetWorldPosition() + moveDirection * 32.f;

	newPosition = {
		std::round(newPosition.x / 32.f) * 32.f,
		std::round(newPosition.y / 32.f) * 32.f
	};

	const auto& currentLevel = GetLevelAtPosition(newPosition);
	return currentLevel != nullptr && currentLevel->CanMoveTo(newPosition);
}

bool WorldDefinition::ParseWorldDefinition(sol::state& lua, const std::filesystem::path& worldDefinitionFilepath)
{
	std::string line, text;
	std::ifstream in(worldDefinitionFilepath);
	while (std::getline(in, line))
	{
		text += line + "\n";
	}

	if (text.empty())
	{
		return false;
	}

	const char* content = text.c_str();

	const size_t content_length = strlen(content);

	hoxml_context_t* hoxml_context = new hoxml_context_t();
	const auto buffer = static_cast<char*>(malloc(content_length * 2));

	hoxml_init(hoxml_context, buffer, content_length * 2);

	std::unordered_map<std::string, std::shared_ptr<Level>> levels;
	std::unordered_map<std::string, std::unordered_map<std::string, Portal>> portals;

	// Loop until the "end of document" code is returned
	hoxml_code_t code = hoxml_parse(hoxml_context, content, content_length);
	while (code != HOXML_END_OF_DOCUMENT)
	{
		if (code == HOXML_ELEMENT_BEGIN)
		{
			if (strcmp("Level", hoxml_context->tag) == 0
				&& !ParseLevel(lua, levels, portals, hoxml_context, content, content_length))
			{
				return false;
			}
		}

		code = hoxml_parse(hoxml_context, content, content_length);
	}

	if (levels.empty())
	{
		std::cerr << "WorldDefinition::ParseWorldDefinition - No levels parsed!\n";
		free(buffer);
		delete hoxml_context;
		return false;
	}

	// If the provided starting level doesn't exist, pick the first parsed level as root
	std::string root = OVERWORLD_ROOT_LEVEL;
	if (levels.find(root) == levels.end())
	{
		root = levels.begin()->first;
		std::cerr << "WorldDefinition::ParseWorldDefinition - start level '" << OVERWORLD_ROOT_LEVEL << "' not found. Using '" << root << "' as root.\n";
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

		const auto& currentLevel = levels.at(current);
		const Level::AdjacentLevels& adjacentLevels = currentLevel->GetAdjacentLevels();
		const sf::Vector2i& currentOrigin = origins.at(current);
		const int curCols = static_cast<int>(currentLevel->GetNumColumns());
		const int curRows = static_cast<int>(currentLevel->GetNumRows());

		auto tryAssignNeighbor = [&](const std::string& neighbourName, const std::function<sf::Vector2i()>& computeOrigin) {
			if (neighbourName.empty())
			{
				return;
			}

			if (levels.find(neighbourName) == levels.end())
			{
				return;
			}

			sf::Vector2i proposedOrigin = computeOrigin();

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
					std::cerr << "WorldDefinition::ParseWorldDefinition - placement conflict for level '" << neighbourName << "'. Existing origin: ("
						<< it->second.x << "," << it->second.y << ") proposed: (" << proposedOrigin.x << "," << proposedOrigin.y << ").\n";
					// Keep the existing placement (could add reconciliation logic here)
				}
			}
			};

		// North neighbour: it sits above current -> neighbour.origin.y = current.origin.y - neighbourRows
		tryAssignNeighbor(adjacentLevels.m_North, [&]() -> sf::Vector2i {
			const auto& neighbor = levels.at(adjacentLevels.m_North);
			return { currentOrigin.x, currentOrigin.y - static_cast<int>(neighbor->GetNumRows()) };
			});

		// South neighbour: it sits below current -> neighbour.origin.y = current.origin.y + currentRows
		tryAssignNeighbor(adjacentLevels.m_South, [&]() -> sf::Vector2i {
			return { currentOrigin.x, currentOrigin.y + curRows };
			});

		// East neighbour: sits to the right -> neighbour.origin.x = current.origin.x + currentCols
		tryAssignNeighbor(adjacentLevels.m_East, [&]() -> sf::Vector2i {
			return { currentOrigin.x + curCols, currentOrigin.y };
			});

		// West neighbour: sits to the left -> neighbour.origin.x = current.origin.x - neighbourCols
		tryAssignNeighbor(adjacentLevels.m_West, [&]() -> sf::Vector2i {
			const auto& neighbor = levels.at(adjacentLevels.m_West);
			return { currentOrigin.x - static_cast<int>(neighbor->GetNumColumns()), currentOrigin.y };
			});
	}

	// Apply the computed offsets to the levels. Some levels are not connected to the root (maybe offshoots, interiors, etc...)
	sf::Vector2i maxExtentStart(-0xFFFF, -0xFFFF);
	for (const auto& [levelName, origin] : origins)
	{
		const auto& levelPtr = levels.at(levelName);
		maxExtentStart.x = std::max<int>(origin.x * 32 + static_cast<int>(levelPtr->GetNumColumns()) * 32, maxExtentStart.x);
		maxExtentStart.y = std::max<int>(origin.y * 32 + static_cast<int>(levelPtr->GetNumRows()) * 32, maxExtentStart.y);
	}
	maxExtentStart.x += 3200;

	for (auto& [levelName, levelPtr] : levels)
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


	// ALL DONE! Store the results...
	m_levels = std::move(levels);
	m_levelPortals = std::move(portals);

	free(buffer);
	delete hoxml_context;
	return true;
}

bool WorldDefinition::ParseLevel(
	sol::state& lua,
	std::unordered_map<std::string,
	std::shared_ptr<Level>>&levels,
	std::unordered_map<std::string,
	std::unordered_map<std::string, Portal>>&portals,
	hoxml_context_t*& context,
	const char* xml,
	const size_t xmlLength)
{
	hoxml_code_t code = HOXML_ELEMENT_BEGIN;

	std::string levelName;
	std::shared_ptr<MapData> tileMapData = nullptr;
	Level::AdjacentLevels adjacentLevels{};

	bool closedLevelTag = false;
	while (!closedLevelTag)
	{
		if (code == HOXML_ELEMENT_BEGIN)
		{
			if (strcmp("Portal", context->tag) == 0)
			{
				if (levelName.empty())
				{
					std::cerr << "WorldDefinition::ParseLevel - Unable to parse Portal as LevelID is empty!\n";
					return false;
				}

				if (!ParsePortal(levelName, portals, context, xml, xmlLength))
				{
					std::cerr << "WorldDefinition::ParseLevel - Error occured whilst parsing the Portal!\n";
					return false;
				}
			}
		}
		else if (code == HOXML_ATTRIBUTE)
		{
			if (strcmp("id", context->attribute) == 0)
			{
				levelName = context->value;
			}
			else if (strcmp("tmj", context->attribute) == 0)
			{
				const std::filesystem::path tmjPath = context->value;
				if (!std::filesystem::exists(tmjPath))
				{
					std::cerr << "WorldDefinition::ParseLevel - TMJ at path " << tmjPath <<
						" doesn't exist! Check the WorldDefinition.xml file\n";
					return false;
				}

				tileMapData = TileParser::ParseTMJ(tmjPath);
				if (tileMapData == nullptr)
				{
					return false;
				}
			}
			else if (strcmp("north", context->attribute) == 0)
			{
				adjacentLevels.m_North = context->value;
			}
			else if (strcmp("south", context->attribute) == 0)
			{
				adjacentLevels.m_South = context->value;
			}
			else if (strcmp("east", context->attribute) == 0)
			{
				adjacentLevels.m_East = context->value;
			}
			else if (strcmp("west", context->attribute) == 0)
			{
				adjacentLevels.m_West = context->value;
			}
		}
		else if (code == HOXML_ELEMENT_END)
		{
			if (strcmp("Level", context->tag) == 0)
			{
				closedLevelTag = true;
			}
		}

		if (!closedLevelTag)
		{
			code = hoxml_parse(context, xml, xmlLength);
		}
	}

	if (levelName.empty())
	{
		std::cerr << "WorldDefinition::ParseLevel - Unable to parse level as ID is empty!\n";
		return false;
	}

	if (tileMapData == nullptr)
	{
		std::cerr << "WorldDefinition::ParseLevel - Unable to parse level " << levelName <<
			"as tileMapData is null. Check the log and try again.\n";
		return false;
	}

	levels[levelName] = std::make_shared<Level>(lua, levelName, tileMapData, adjacentLevels);
	return true;
}

bool WorldDefinition::ParsePortal(const std::string& levelName,
	std::unordered_map<std::string, std::unordered_map<std::string, Portal>>& portals,
	hoxml_context_t*& context,
	const char* xml,
	const size_t xmlLength)
{
	Portal p;

	hoxml_code_t code = HOXML_ELEMENT_BEGIN;
	while (code != HOXML_END_OF_DOCUMENT)
	{
		if (code == HOXML_ATTRIBUTE)
		{
			if (strcmp("id", context->attribute) == 0)
			{
				p.m_Name = context->value;
			}
			else if (strcmp("targetLevel", context->attribute) == 0)
			{
				p.m_TargetLevel = context->value;
			}
			else if (strcmp("targetSpawnPoint", context->attribute) == 0)
			{
				p.m_TargetSpawnPoint = context->value;
			}
		}
		else if (code == HOXML_ELEMENT_END)
		{
			if (strcmp("Portal", context->tag) == 0)
			{
				break;
			}
		}

		code = hoxml_parse(context, xml, xmlLength);
	}

	if (p.m_Name.empty())
	{
		std::cerr << "WorldDefinition::ParsePortal - Failed to parse id for Portal in level " << levelName <<
			". Check the WorldDefinition XML and try again\n";
		return false;
	}

	if (p.m_TargetLevel.empty())
	{
		std::cerr << "WorldDefinition::ParsePortal - Failed to parse targetLevel for Portal in level " << levelName <<
			". Check the WorldDefinition XML and try again\n";
		return false;
	}

	if (p.m_TargetSpawnPoint.empty())
	{
		std::cerr << "WorldDefinition::ParsePortal - Failed to parse targetSpawnPoint for Portal in level " << levelName
			<< ". Check the WorldDefinition XML and try again\n";
		return false;
	}

	portals[levelName][p.m_Name] = p;
	return true;
}
