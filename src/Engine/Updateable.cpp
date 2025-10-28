#include "Updateable.h"

#include <iostream>

UpdateableRegistry UPDATEABLE_REGISTRY{};

namespace
{
	uint32_t ID_COUNTER = 0x69;
}

Updateable::Updateable() :
	m_id(ID_COUNTER++)
{
	UPDATEABLE_REGISTRY.RegisterUpdateableInstance(this);
}

Updateable::~Updateable()
{
	UPDATEABLE_REGISTRY.UnregisterUpdateableInstance(m_id);
}

bool UpdateableRegistry::RegisterUpdateableInstance(Updateable* u)
{
	if (m_instances.find(u->m_id) != m_instances.end())
	{
		std::cout << "Updateable::UpdateableRegistry::RegisterUpdateableInstance: Error registering updateable instance - instance with ID " << u->m_id << "already registered!";
		return false;
	}

	m_instances[u->m_id] = u;
	return true;
}

bool UpdateableRegistry::UnregisterUpdateableInstance(const uint32_t id)
{
	m_instances.erase(id);
	return true;
}

void UpdateableRegistry::UpdateAll(float deltaTime)
{
	for (const auto& [_, instance] : m_instances)
	{
		instance->Update(deltaTime);
	}
}
