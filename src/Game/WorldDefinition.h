#ifndef WORLDDEFINITION_H
#define WORLDDEFINITION_H
#include <filesystem>
#include <hoxml.h>
#include <optional>
#include <string>
#include <unordered_map>

#include "Level.h"

class WorldDefinition
{
public:
	WorldDefinition(const std::filesystem::path& worldDefinitionFilepath);

	struct LevelTransition
	{
		std::string m_NewLevelName;
		std::string m_SpawnPointName;
	};

	std::optional<LevelTransition> EnterPortal(const std::string& levelName, const std::string& portalName) const;

	struct Portal
	{
		std::string m_Name;
		std::string m_TargetLevel;
		std::string m_TargetSpawnPoint;
	};

	const Portal& GetPortalData(const std::string& levelName, const std::string& portalName);

	std::vector<std::shared_ptr<Level>> GetLevelsIntersectingRect(const sf::FloatRect& rect) const;

	std::shared_ptr<Level> GetLevel(const std::string& name) const;
	std::shared_ptr<Level> GetLevelAtPosition(const sf::Vector2f& position) const;

	const Level::AdjacentLevels& GetAdjacentLevels(const std::string& levelName) const;

private:
	std::unordered_map<std::string, std::shared_ptr<Level>> m_levels;

	// Key - LevelName
	// Value - Map of Portal Names to Portal data
	std::unordered_map<std::string, std::unordered_map<std::string, Portal>> m_levelPortals;

	bool ParseWorldDefinition(const std::filesystem::path& worldDefinitionFilepath);

	static bool ParseLevel(std::unordered_map<std::string, std::shared_ptr<Level>>& levels, std::unordered_map<std::string, std::unordered_map<std::string, Portal>>& portals, hoxml_context_t*& context, const char* xml, size_t xmlLength);

	static bool ParsePortal(const std::string& levelName, std::unordered_map<std::string, std::unordered_map<std::string, Portal>>& portals, hoxml_context_t*& context, const char* xml, size_t xmlLength);
};

#endif // WORLDDEFINITION_H
