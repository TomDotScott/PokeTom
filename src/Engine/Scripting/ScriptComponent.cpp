#include "ScriptComponent.h"

#include <filesystem>

#include "../Asserts.h"
#include "../Entity.h"


ScriptComponent::ScriptComponent(Entity* owner, sol::state& lua, const std::string& scriptPath) :
	m_owner{ owner },
	m_self{ lua.create_table() },
	m_environment{ sol::environment(lua, sol::create, lua.globals()) }
{
	ASSERT(std::filesystem::exists(scriptPath));

	const auto result = lua.safe_script_file(scriptPath, m_environment);
	ASSERT(result.valid());

	const sol::function init = m_environment["init"];
	if (init.valid())
	{
		init(m_self);
	}

	m_self["LOCAL_ENTITY_ID"] = owner->GetID();

	m_onUpdate = m_self["update"];
	ASSERT_MSG(m_onUpdate.valid(), "update function is missing from script %s!", scriptPath.c_str());

	m_onActivate = m_self["onActivate"];
	ASSERT_MSG(m_onActivate.valid(), "onActivate function is missing from script %s!", scriptPath.c_str());

	m_onDeactivate = m_self["onDeactivate"];
	ASSERT_MSG(m_onDeactivate.valid(), "onDeactivate function is missing from script %s!", scriptPath.c_str());

	m_onCreated = m_self["onCreated"];
	ASSERT_MSG(m_onCreated.valid(), "onCreate function is missing from script %s!", scriptPath.c_str());

	m_onDestroyed = m_self["onDestroyed"];
	ASSERT_MSG(m_onDestroyed.valid(), "onDestroy function is missing from script %s!", scriptPath.c_str());


	m_onCreated(m_self);
}

void ScriptComponent::Update(const float deltaTime)
{
	if (!m_owner->IsActive())
	{
		return;
	}

	ASSERT(m_onUpdate.valid());

	const auto res = m_onUpdate(m_self, deltaTime);
	if (!res.valid())
	{
		sol::error err = res;
		ASSERT_MSG(false, "ScriptComponent update error: %s", err.what());
	}
}

void ScriptComponent::OnActivate()
{
	IUpdateable::OnActivate();

	ASSERT(m_onActivate.valid());

	const auto res = m_onActivate(m_self);
	if (!res.valid())
	{
		const sol::error err = res;
		ASSERT_MSG(false, "OnActivate script error: %s", err.what());
	}
}

void ScriptComponent::OnDeactivate()
{
	IUpdateable::OnDeactivate();

	ASSERT(m_onDeactivate.valid());

	const auto res = m_onDeactivate(m_self);
	if (!res.valid())
	{
		const sol::error err = res;
		ASSERT_MSG(false, "OnDeactivate script error: %s", err.what());
	}
}

void ScriptComponent::OnDestroyed()
{
	IUpdateable::OnDestroyed();

	ASSERT(m_onDestroyed.valid());

	const auto res = m_onDestroyed(m_self);
	if (!res.valid())
	{
		const sol::error err = res;
		ASSERT_MSG(false, "OnDestroyed script error: %s", err.what());
	}
}
