#ifndef SCRIPTED_ENTITY_H
#define SCRIPTED_ENTITY_H
#include "Entity.h"
#include <sol/sol.hpp>

class ScriptedEntity : public Entity
{
public:
	ScriptedEntity(const WorldDefinition* gameWorld, const EntityAnimation& animation);

	void Update(float deltaTime) override;

private:
	// TODO: Make this load from a file location
	sol::state m_script;
};


#endif
