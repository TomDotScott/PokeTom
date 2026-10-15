#include "Player.h"

#include "DialogueBox.h"
#include "FollowEntityComponent.h"
#include "../Engine/Asserts.h"
#include "../Engine/EntityRegistry.h"
#include "../Engine/CodeGen/Resources.hpp"
#include "../Engine/Globals.h"
#include "../Engine/Animation/AnimationComponent.h"
#include "../Engine/Input/Keyboard.h"
#include "Monsters/MonsterOverworldAnimationComponent.h"
#include "Monsters/MonsterPartyComponent.h"
#include "Monsters/PocketMonsterManager.h"


Player::Player(EntityRegistry& entities, const sf::Vector2f& position) :
	Entity(position, { 1.f, 1.f }),
	m_followerEntityID(~0U),
	m_entities(entities)
{
	AddComponent<GridMovementComponent>(this, 32.f, 3.f, 6.5f);
	AddComponent<EntityAnimationComponent>(this, "PLAYER_BOY", EntityAnimationComponent::eAnimationName::IDLE_DOWN);
	m_mapper.Map(UP, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::W));
	m_mapper.Map(DOWN, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::S));

	m_mapper.Map(LEFT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::A));
	m_mapper.Map(RIGHT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::D));

	m_mapper.Map(SPRINT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::LShift));

	m_mapper.Map(INTERACT, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::E));

	m_mapper.Map(ADD_FOLLOWER, eInputType::Keyboard, static_cast<int>(sf::Keyboard::Key::F));

	m_mapper.OnButtonPressed(ADD_FOLLOWER, [this]() {
		if (m_followerEntityID == ~0U)
		{
			SpawnFollowerEntity();
		}
		else
		{
			RemoveFollowerEntity();
		}
	});


	auto& mpc = AddComponent<MonsterPartyComponent>(
		this,
		entities,
		std::vector<MonsterPartyComponent::MonsterPartyInfo>{
			{
				.m_Monster = PocketMonsterManager::Get()->GetMonsterDetails(403),
				.m_Level = 10
			},
			{
				.m_Monster = PocketMonsterManager::Get()->GetMonsterDetails(507),
				.m_Level = 12
			}
		}
	);

	mpc.OnActivate();

	SetPosition(position);

	// The player is active by default
	Entity::OnActivate();
}

void Player::Move()
{
	GridMovementComponent* movement = GetComponent<GridMovementComponent>();
	ASSERT_MSG(movement != nullptr, "No GridMovementComponent attached to the Player!");

	if (!movement->IsMoving())
	{
		if (m_mapper.IsButtonDown(UP))
		{
			movement->Move(eDirection::North);
		}
		else if (m_mapper.IsButtonDown(DOWN))
		{
			movement->Move(eDirection::South);
		}
		else if (m_mapper.IsButtonDown(LEFT))
		{
			movement->Move(eDirection::West);
		}
		else if (m_mapper.IsButtonDown(RIGHT))
		{
			movement->Move(eDirection::East);
		}
	}

	movement->SetSprinting(m_mapper.IsButtonDown(SPRINT));
}

void Player::SpawnFollowerEntity()
{
	ASSERT(m_followerEntityID == ~0U);

	Entity& followEntity = m_entities.Create();

	m_followerEntityID = followEntity.GetID();

	const MonsterPartyComponent* partyComp = GetComponent<MonsterPartyComponent>();
	const PocketMonsterEntity* firstMonster = partyComp->GetActiveMonster();

	followEntity.AddComponent<MonsterOverworldAnimationComponent>(&followEntity,
	                                                              firstMonster->GetMonsterID(),
	                                                              eMonsterOverworldState::Walking,
	                                                              MonsterOverworldAnimationComponent::eAnimationName::DOWN
	);
	followEntity.AddComponent<EntityFollowComponent>(&followEntity, m_entities, GetID());

	// Set the position of the entity to behind the player
	const GridMovementComponent* myGMC = GetComponent<GridMovementComponent>();
	const eDirection myPreviousDirection = myGMC->GetPreviousDirection();
	eDirection spawnDirection = eDirection::North;
	switch (myPreviousDirection) {
	case eDirection::None:
		spawnDirection = eDirection::North;
		break;
	case eDirection::North:
		spawnDirection = eDirection::South;
		break;
	case eDirection::South:
		spawnDirection = eDirection::North;
		break;
	case eDirection::East:
		spawnDirection = eDirection::West;
		break;
	case eDirection::West:
		spawnDirection = eDirection::East;
		break;
	}
	const sf::Vector2f myPreviousPos = myGMC->GetNextPosition(spawnDirection);

	GridMovementComponent* followGMC = followEntity.GetComponent<GridMovementComponent>();

	followGMC->SetCanMoveCallback([&](const Entity* ent, const eDirection dir)
		{
			return true;
		});

	followGMC->SetWorldPosition(myPreviousPos);

	followEntity.OnActivate();
}

void Player::RemoveFollowerEntity()
{
	Entity* followEntity = m_entities.Get(m_followerEntityID);
	if (followEntity != nullptr)
	{
		followEntity->OnDeactivate();

		m_entities.Destroy(m_followerEntityID);
	}
	m_followerEntityID = ~0U;
}


void Player::Update(const float deltaTime)
{
	// Update inputs
	m_mapper.Update();

	if (!DialogueBox::IsVisible())
	{
		Move();
	}

	Entity::Update(deltaTime);
}

void Player::SetCanMoveCallback(const can_move_func& callback)
{
	GetComponent<GridMovementComponent>()->SetCanMoveCallback(callback);
}

void Player::AddOnInteractPressedCallback(const std::function<void()>& callback)
{
	m_mapper.OnButtonPressed(INTERACT, callback);
}
