#ifndef LUAREGISTRY_H
#define LUAREGISTRY_H

#include <sol/sol.hpp>


// Forward decs for the various APIs that will be exposed to the scripting
// TODO: Add Input, Rendering, and other system code here
class EntityRegistry;
class WorldDefinition;

class LuaRegistry
{
public:
	LuaRegistry(sol::state& lua, WorldDefinition& world, EntityRegistry& entities);

private:
	void RegisterEntityAPI(sol::state& lua, WorldDefinition& world, EntityRegistry& entities);
	void RegisterDialogueAPI(sol::state& lua);
	void RegisterDirections(sol::state& lua);
	void RegisterAnimationNames(sol::state& lua);
};

#endif
