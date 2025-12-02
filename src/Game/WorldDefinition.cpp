#include "WorldDefinition.h"

#include <fstream>
#include <iostream>

#include "Level.h"
#include "TileParser.h"


WorldDefinition::WorldDefinition(const std::filesystem::path& worldDefinitionFilepath, std::string startLevel) :
	m_currentLevel(std::move(startLevel))
{
	ParseWorldDefinition(worldDefinitionFilepath);
}

std::optional<WorldDefinition::LevelTransition> WorldDefinition::EnterPortal(const std::string& portalName)
{
	const auto& currentLevelPortals = m_levelPortals.at(m_currentLevel);
	if (currentLevelPortals.find(portalName) == currentLevelPortals.end())
	{
		std::cerr << "WorldDefinition::OnPlayerEnterPortal - Portal with name " << portalName <<
			" Does not exist in level " << m_currentLevel << "\n";
		return std::nullopt;
	}

	const Portal& portalData = currentLevelPortals.at(portalName);
	m_currentLevel = portalData.m_TargetLevel;

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

const std::string& WorldDefinition::GetCurrentLevelName() const
{
	return m_currentLevel;
}

std::shared_ptr<Level> WorldDefinition::GetLevel(const std::string& name) const
{
	if (m_levels.find(name) == m_levels.end())
	{
		return nullptr;
	}

	return m_levels.at(name);
}

const Level::AdjacentLevels& WorldDefinition::GetAdjacentLevels(const std::string& levelName) const
{
	return GetLevel(levelName)->GetAdjacentLevels();
}

bool WorldDefinition::ParseWorldDefinition(const std::filesystem::path& worldDefinitionFilepath)
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
				&& !ParseLevel(levels, portals, hoxml_context, content, content_length))
			{
				return false;
			}
		}

		code = hoxml_parse(hoxml_context, content, content_length);
	}

	m_levels = levels;
	m_levelPortals = portals;

	free(buffer);
	delete hoxml_context;
	return true;
}

bool WorldDefinition::ParseLevel(std::unordered_map<std::string, std::shared_ptr<Level>>& levels,
	std::unordered_map<std::string, std::unordered_map<std::string, Portal>>& portals,
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

	// Work out the offset from the levels that are at the bottom or left of this level (hopefully they all load in order!)
	// TODO: Work out a way of determining the offset without having to load the levels in a specific way - maybe some sort of tree structure that can update dynamically?
	uint32_t offsetCols = 0;
	uint32_t offsetRows = 0;
	if (!adjacentLevels.m_South.empty())
	{
		const std::shared_ptr<Level>& southLevel = levels.at(adjacentLevels.m_South);
		offsetRows = southLevel->GetOffsetFromOrigin().y + tileMapData->m_NumRows;
	}

	if (!adjacentLevels.m_West.empty())
	{
		const std::shared_ptr<Level>& westLevel = levels.at(adjacentLevels.m_South);
		offsetCols = westLevel->GetOffsetFromOrigin().x + tileMapData->m_NumColumns;
	}

	levels[levelName] = std::make_shared<Level>(tileMapData, adjacentLevels, offsetRows, offsetCols);
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
