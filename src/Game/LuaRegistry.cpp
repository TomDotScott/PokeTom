#if !BUILD_MASTER
#pragma optimize("", off)
#endif
#include "LuaRegistry.h"

#include <iostream>
#include <SFML/System/Vector2.hpp>

#include "DialogueBox.h"
#include "DialogueComponent.h"
#include "Player.h"
#include "WorldDefinition.h"
#include "../Engine/Asserts.h"
#include "../Engine/Entity.h"
#include "../Engine/EntityRegistry.h"
#include "../Engine/GridMovementComponent.h"
#include "../Engine/Animation/AnimationComponent.h"
#include "../Engine/Scripting/LuaBindingMacros.h"
#include "../Engine/Scripting/ScriptComponent.h"


LuaRegistry::LuaRegistry(sol::state& lua, WorldDefinition& world, EntityRegistry& entities)
{
	RegisterEntityAPI(lua, world, entities);
	RegisterDialogueAPI(lua);
	RegisterDirections(lua);
	RegisterAnimationNames(lua);
}

void LuaRegistry::RegisterEntityAPI(sol::state& lua, WorldDefinition& world, EntityRegistry& entities)
{
	LUA_API_BEGIN(Entity, lua)
		LUA_FUNC(Entity, Create,
			[&](const float x, const float y) -> uint32_t {
				// TODO: I need a reliable way to get the player!
				const std::shared_ptr<Level> level = world.LastTransitionedToLevel();

				ASSERT(level != nullptr);

				const Entity& e = entities.Create<Entity>(static_cast<sf::Vector2f>(level->GetWorldOrigin()) + sf::Vector2f{ x, y });
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

		LUA_FUNC(Entity, TurnToFace,
			[&entities](const uint32_t entityID, const uint32_t facingEntityID) {
				auto* e = entities.Get<Entity>(entityID);

				if (!e->IsActive())
				{
					return;
				}

				auto* other = entities.Get<Entity>(facingEntityID);
				if (!other)
				{
					ASSERT(false);
					return;
				}

				const auto omc = other->GetComponent<GridMovementComponent>();
				const auto mc = e->GetComponent<GridMovementComponent>();

				if (omc == nullptr || mc == nullptr)
				{
					return;
				}

				const eDirection otherDirection = omc->GetCurrentDirection() == eDirection::None ? omc->GetPreviousDirection() : omc->GetCurrentDirection();

				eDirection newDirection = eDirection::None;
				if (otherDirection == eDirection::East)
				{
					newDirection = eDirection::West;
				}
				else if (otherDirection == eDirection::West)
				{
					newDirection = eDirection::East;
				}
				else if (otherDirection == eDirection::North)
				{
					newDirection = eDirection::South;
				}
				else if (otherDirection == eDirection::South)
				{
					newDirection = eDirection::North;
				}
				else
				{
					ASSERT(false);
				}

				mc->SetDirection(newDirection);
			})

		LUA_FUNC(Entity, Move,
			[&entities](const uint32_t id, const int direction)
			{
				const auto e = entities.Get<Entity>(id);
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
						return world.CanMoveTo(ent, entities, dir);
					});
			})

		LUA_FUNC(Entity, AddAnimationComponent,
			[&entities](const uint32_t id, const std::string& animationPath, const int initialAnimation)
			{
				Entity* e = entities.Get<Entity>(id);

				const auto initialAnimName = static_cast<EntityAnimationComponent::eAnimationName>(initialAnimation);

				// TODO: Fix these magic numbers!
				e->AddComponent<EntityAnimationComponent>(e, animationPath, initialAnimName);
			})

		// TODO: Make this a script name or ID instead of a filepath
		LUA_FUNC(Entity, AddScriptComponent,
			[&](const uint32_t id, const std::string& scriptPath)
			{
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

		LUA_FUNC(Entity, AddDialogueComponent,
			[&entities](const uint32_t id, const std::string& dialogueID, bool shouldLoop, float loopTimer)
			{
				printf("Adding Dialogue Component! %d %s %s %f\n", id, dialogueID.c_str(), shouldLoop ? "true" : "false", loopTimer);

				Entity* e = entities.Get<Entity>(id);

				if (e == nullptr)
				{
					return;
				}

				auto& c = e->AddComponent<DialogueComponent>(e, dialogueID, shouldLoop, loopTimer);

				if (e->IsActive())
				{
					c.OnActivate();
				}
			})

		LUA_FUNC(Entity, HasDialogueLeft,
			[&entities](const uint32_t id) -> bool
			{
				Entity* e = entities.Get<Entity>(id);

				if (e == nullptr)
				{
					return false;
				}

				if (!e->IsActive())
				{
					return false;
				}

				const auto* c = e->GetComponent<DialogueComponent>();
				if (c == nullptr)
				{
					return false;
				}

				return c->HasDialogueLeft();
			})

	LUA_API_END()
}

void LuaRegistry::RegisterDialogueAPI(sol::state& lua)
{
	LUA_API_BEGIN(Dialogue, lua)
		LUA_FUNC(Dialogue, Show,
			[]() -> void {
				DialogueBox::SetVisible(true);
			})

		LUA_FUNC(Dialogue, SetText,
			[](const std::string& text) -> void {
				// TODO: Figure out how tf I am going to do formatted text here!
				DialogueBox::SetText(text.c_str());
			})

		LUA_FUNC(Dialogue, Hide,
			[]() -> void {
				DialogueBox::SetVisible(false);
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
		LUA_CONST(AnimationName, IDLE_UP, static_cast<int>(EntityAnimationComponent::WALK_DOWN))
		LUA_CONST(AnimationName, IDLE_DOWN, static_cast<int>(EntityAnimationComponent::IDLE_DOWN))
		LUA_CONST(AnimationName, IDLE_LEFT, static_cast<int>(EntityAnimationComponent::IDLE_LEFT))
		LUA_CONST(AnimationName, IDLE_RIGHT, static_cast<int>(EntityAnimationComponent::IDLE_RIGHT))

		LUA_CONST(AnimationName, WALK_UP, static_cast<int>(EntityAnimationComponent::WALK_UP))
		LUA_CONST(AnimationName, WALK_DOWN, static_cast<int>(EntityAnimationComponent::WALK_DOWN))
		LUA_CONST(AnimationName, WALK_LEFT, static_cast<int>(EntityAnimationComponent::WALK_LEFT))
		LUA_CONST(AnimationName, WALK_RIGHT, static_cast<int>(EntityAnimationComponent::WALK_RIGHT))

		LUA_CONST(AnimationName, RUN_UP, static_cast<int>(EntityAnimationComponent::RUN_UP))
		LUA_CONST(AnimationName, RUN_DOWN, static_cast<int>(EntityAnimationComponent::RUN_DOWN))
		LUA_CONST(AnimationName, RUN_LEFT, static_cast<int>(EntityAnimationComponent::RUN_LEFT))
		LUA_CONST(AnimationName, RUN_RIGHT, static_cast<int>(EntityAnimationComponent::RUN_RIGHT))

		LUA_API_END()
}
