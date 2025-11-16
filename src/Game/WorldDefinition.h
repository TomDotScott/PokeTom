#ifndef WORLDDEFINITION_H
#define WORLDDEFINITION_H
#include <filesystem>
#include <hoxml.h>
#include <optional>
#include <string>
#include <unordered_map>

class Level;

class WorldDefinition
{
public:
	WorldDefinition(const std::filesystem::path& worldDefinitionFilepath, std::string startLevel);

	struct LevelTransition
	{
		std::string m_NewLevelName;
		std::string m_SpawnPointName;
	};

	std::optional<LevelTransition> EnterPortal(const std::string& portalName);

	struct Portal
	{
		std::string m_Name;
		std::string m_TargetLevel;
		std::string m_TargetSpawnPoint;
	};

	const Portal& GetPortalData(const std::string& levelName, const std::string& portalName);

	const std::string& GetCurrentLevelName() const;
	std::shared_ptr<Level> GetLevel(const std::string& name) const;

private:
	std::string m_currentLevel;
	std::unordered_map<std::string, std::shared_ptr<Level>> m_levels;


	// Key - LevelName
	// Value - Map of Portal Names to Portal data
	std::unordered_map<std::string, std::unordered_map<std::string, Portal>> m_levelPortals;

	bool ParseWorldDefinition(const std::filesystem::path& worldDefinitionFilepath);

	static bool ParseLevel(std::unordered_map<std::string, std::shared_ptr<Level>>& levels, std::unordered_map<std::string, std::unordered_map<std::string, Portal>>& portals, hoxml_context_t*& context, const char* xml, size_t xmlLength);

	static bool ParsePortal(const std::string& levelName, std::unordered_map<std::string, std::unordered_map<std::string, Portal>>& portals, hoxml_context_t*& context, const char* xml, size_t xmlLength);
};

#endif // WORLDDEFINITION_H
