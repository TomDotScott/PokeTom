#if !BUILD_MASTER
#pragma optimize("", off)
#endif
#include "LuaRegistry.h"
#include "../Engine/EntityRegistry.h"
#include "../Engine/Entity.h"
#include "../Engine/GridMovementComponent.h"
#include "../Engine/Animation/AnimationComponent.h"
#include "../Engine/Scripting/LuaBindingMacros.h"
#include <SFML/System/Vector2.hpp>

#include "WorldDefinition.h"
#include "../Engine/Scripting/ScriptComponent.h"


LuaRegistry::LuaRegistry(sol::state& lua, WorldDefinition& world, EntityRegistry& entities)
{
	RegisterEntityAPI(lua, world, entities);
	RegisterDirections(lua);
	RegisterAnimationNames(lua);
}

void LuaRegistry::RegisterEntityAPI(sol::state& lua, WorldDefinition& world, EntityRegistry& entities)
{
	LUA_API_BEGIN(Entity, lua)
		LUA_FUNC(Entity, Create,
			[&entities](const float x, const float y) -> uint32_t {
				const Entity& e = entities.Create<Entity>(sf::Vector2f{ x, y });
				return e.GetID();
			})

		LUA_FUNC(Entity, Destroy,
			[&entities](const uint32_t id)
			{
				entities.Destroy(id);
			})

		LUA_FUNC(Entity, SetActive,
			[&entities](const uint32_t id, const bool active)
			{
				auto* e = entities.Get<Entity>(id);
				if (active)
				{
					e->OnActivate();
				}
				else
				{
					e->OnDeactivate();
				}
			})

		LUA_FUNC(Entity, Move,
			[&entities](const uint32_t id, const int direction)
			{
				auto e = entities.Get<Entity>(id);
				if (e == nullptr)
				{
					std::cerr << "Entity with ID " << id << " returned null!\n";
					return;
				}

				auto movement = e->GetComponent<GridMovementComponent>();
				if (movement == nullptr)
				{
					// std::cerr << "Entity with ID " << id << " does not have a GridMovementComponent attached!\n";
					return;
				}

				movement->Move(static_cast<eDirection>(direction));
			})

		LUA_FUNC(Entity, CanMove,
			[&entities](const uint32_t id, const int direction)
			{
				auto e = entities.Get<Entity>(id);
				if (e == nullptr)
				{
					// std::cerr << "Entity with ID " << id << " returned null!\n";
					return false;
				}

				auto movement = e->GetComponent<GridMovementComponent>();
				if (movement == nullptr)
				{
					return false;
				}

				return movement->CanMove(static_cast<eDirection>(direction));
			})

		LUA_FUNC(Entity, AddGridMovementComponent,
			[&](const uint32_t id)
			{
				Entity* e = entities.Get<Entity>(id);
				// TODO: Fix these magic numbers!
				auto& c = e->AddComponent<GridMovementComponent>(e, 32.f, 3.f, 6.f);

				c.SetWorldPosition(e->GetPosition());

				c.SetCanMoveCallback([&](Entity* ent, const eDirection dir)
				{
						return world.CanMoveTo(ent, dir);
				});
			})

	LUA_FUNC(Entity, AddAnimationComponent,
			[&entities](const uint32_t id, const std::string& animationPath, const int initialAnimation)
			{
				Entity* e = entities.Get<Entity>(id);

				// TODO: This is horrible!
				EntityAnimationComponent::eAnimationName initialAnimName;
				if (initialAnimation == "IDLE_UP")
				{
					initialAnimName = EntityAnimationComponent::IDLE_UP;
				}
				else if (initialAnimation == "IDLE_DOWN")
				{
					initialAnimName = EntityAnimationComponent::IDLE_DOWN;
				}
				else if (initialAnimation == "IDLE_LEFT")
				{
					initialAnimName = EntityAnimationComponent::IDLE_LEFT;
				}
				else if (initialAnimation == "IDLE_RIGHT")
				{
					initialAnimName = EntityAnimationComponent::IDLE_RIGHT;
				}
				else if (initialAnimation == "WALK_UP")
				{
					initialAnimName = EntityAnimationComponent::WALK_UP;
				}
				else if (initialAnimation == "WALK_DOWN")
				{
					initialAnimName = EntityAnimationComponent::WALK_DOWN;
				}
				else if (initialAnimation == "WALK_LEFT")
				{
					initialAnimName = EntityAnimationComponent::WALK_LEFT;
				}
				else if (initialAnimation == "WALK_RIGHT")
				{
					initialAnimName = EntityAnimationComponent::WALK_RIGHT;
				}
				else if (initialAnimation == "RUN_UP")
				{
					initialAnimName = EntityAnimationComponent::RUN_UP;
				}
				else if (initialAnimation == "RUN_DOWN")
				{
					initialAnimName = EntityAnimationComponent::RUN_DOWN;
				}
				else if (initialAnimation == "RUN_LEFT")
				{
					initialAnimName = EntityAnimationComponent::RUN_LEFT;
				}
				else if (initialAnimation == "RUN_RIGHT")
				{
					initialAnimName = EntityAnimationComponent::RUN_RIGHT;
				}
				else
				{
					std::cerr << "Unable to map " << initialAnimation << " to the eAnimationName enum!\n!";
					initialAnimName = EntityAnimationComponent::IDLE_DOWN;
				}

				// TODO: Fix these magic numbers!
				e->AddComponent<EntityAnimationComponent>(e, animationPath, initialAnimName);
			})

	// TODO: Make this a script name or ID instead of a filepath
	LUA_FUNC(Entity, AddScriptComponent,
		[&](const uint32_t id, const std::string& scriptPath){
			Entity* e = entities.Get<Entity>(id);

			if (e == nullptr)
			{
				return;
			}

			auto& c = e->AddComponent<ScriptComponent>(e, lua, scriptPath);

			// If the entity is already active in the scene, make sure the script onActivate is called
			if (e->IsActive())
			{
				c.OnActivate();
			}
		})

	LUA_API_END()
}

void LuaRegistry::RegisterDirections(sol::state& lua)
{
	LUA_API_BEGIN(Direction, lua)
		LUA_CONST(Direction, North, static_cast<int>(eDirection::North))
		LUA_CONST(Direction, South, static_cast<int>(eDirection::South))
		LUA_CONST(Direction, East, static_cast<int>(eDirection::East))
		LUA_CONST(Direction, West, static_cast<int>(eDirection::West))
	LUA_API_END()
}

void LuaRegistry::RegisterAnimationNames(sol::state& lua)
{
	LUA_API_BEGIN(AnimationName, lua)
		LUA_CONST(AnimationName, IDLE_UP, "IDLE_UP")
		LUA_CONST(AnimationName, IDLE_DOWN, "IDLE_DOWN")
		LUA_CONST(AnimationName, IDLE_LEFT, "IDLE_LEFT")
		LUA_CONST(AnimationName, IDLE_RIGHT, "IDLE_RIGHT")

		LUA_CONST(AnimationName, WALK_UP, "WALK_UP")
		LUA_CONST(AnimationName, WALK_DOWN, "WALK_DOWN")
		LUA_CONST(AnimationName, WALK_LEFT, "WALK_LEFT")
		LUA_CONST(AnimationName, WALK_RIGHT, "WALK_RIGHT")

		LUA_CONST(AnimationName, RUN_UP, "RUN_UP")
		LUA_CONST(AnimationName, RUN_DOWN, "RUN_DOWN")
		LUA_CONST(AnimationName, RUN_LEFT, "RUN_LEFT")
		LUA_CONST(AnimationName, RUN_RIGHT, "RUN_RIGHT")

	LUA_API_END()
}
