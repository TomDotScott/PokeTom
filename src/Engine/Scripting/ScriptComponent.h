#ifndef SCRIPTCOMPONENT_H
#define SCRIPTCOMPONENT_H
#include "../IUpdateable.h"
#include <sol/sol.hpp>

class Entity;

class ScriptComponent : public IUpdateable
{
public:
	ScriptComponent(Entity* owner, sol::state& lua, const std::string& scriptPath);

	void Update(float deltaTime) override;

	void OnActivate() override;
	void OnDeactivate() override;

	void OnDestroyed() override;

	void OnPlayerInteractPressed() override;

	void SetVariableValue(const std::string& variableName, const std::string& variableValue);

private:
	Entity* m_owner;
	sol::table m_self;

	sol::environment m_environment;
	sol::function m_onUpdate;

	sol::function m_onCreated;
	sol::function m_onDestroyed;

	sol::function m_onActivate;
	sol::function m_onDeactivate;

	sol::function m_onPlayerInteract;
};

#endif
