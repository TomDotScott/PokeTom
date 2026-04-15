#ifndef WORLDDEFINITION_H
#define WORLDDEFINITION_H
#include <filesystem>
#include <hoxml.h>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <sol/sol.hpp>

#include "Level.h"
#include "../Engine/GridMovementComponent.h"
#include "../Engine/ISerialisable.h"


class EntityRegistry;

class WorldDefinition : public ISerialisable
{
public:
	WorldDefinition(sol::state& lua, const std::filesystem::path& worldDefinitionFilepath);

	struct LevelTransition
	{
		std::string m_NewLevelName;
		std::string m_SpawnPointName;
	};

	std::optional<LevelTransition> EnterPortal(const std::string& levelName, const std::string& portalName) const;
	void LoadLevelScripts(sol::state& lua);

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

	bool CanMoveTo(const Entity* entity, EntityRegistry& entities, eDirection direction) const;
	void SortLevels();
	bool LoadFromXML(const XmlNode& node) override;

private:
	std::unordered_map<std::string, std::shared_ptr<Level>> m_levels;

	// Key - LevelName
	// Value - Map of Portal Names to Portal data
	std::unordered_map<std::string, std::unordered_map<std::string, Portal>> m_levelPortals;

	sol::state* m_lua;
};

#endif // WORLDDEFINITION_H
