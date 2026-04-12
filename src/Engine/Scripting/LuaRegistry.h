#ifndef LUAREGISTRY_H
#define LUAREGISTRY_H

#include <sol/sol.hpp>


// Forward decs for the various APIs that will be exposed to the scripting
// TODO: Add Input, Rendering, and other system code here
class EntityRegistry;

class LuaRegistry
{
public:
	LuaRegistry(sol::state& lua, EntityRegistry& entities);

private:
	void RegisterEntityAPI(sol::state& lua, EntityRegistry& entities);
	void RegisterDirections(sol::state& lua);
	void RegisterAnimationNames(sol::state& lua);
};

#endif
