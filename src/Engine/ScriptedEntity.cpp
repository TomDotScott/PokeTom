#include "ScriptedEntity.h"

#include "../Game/GridMovementComponent.h"


ScriptedEntity::ScriptedEntity(const WorldDefinition* gameWorld, const EntityAnimation& animation) :
	Entity{ gameWorld, animation }
{
	m_script.open_libraries(sol::lib::base);

	m_script.script(""
		"direction = 0\n"
		"function update (dt)\n"
		"--  print(\"Updating \"..dt)\n"
		"  if CAN_MOVE (direction) then\n"
		"    MOVE(direction)\n"
		"  else\n"
		"    if direction == 0 then\n"
		"      direction = 1\n"
		"    else\n"
		"      direction = 0\n"
		"    end\n"
		"  end\n"
		"  return 1\n"
		"end"
	);

	// TODO: Is there a better / more standard way of doing this? Maybe some Macro magick to auto generate bindings?
	m_script["CAN_MOVE"] = [&](int direction) -> bool{
		return CanMove(static_cast<GridMovementComponent::eDirection>(direction));
	};

	m_script["MOVE"] = [&](int direction){
		Move(static_cast<GridMovementComponent::eDirection>(direction));
	};
}

void ScriptedEntity::Update(const float deltaTime)
{
	int returnCode = m_script["update"](deltaTime);

	Entity::Update(deltaTime);
}
