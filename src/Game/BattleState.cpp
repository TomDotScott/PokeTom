#include "BattleState.h"

#include <iostream>

#include "DialogueBox.h"
#include "GameEvents.h"
#include "MoveComponent.h"
#include "../Engine/Stringtable.h"
#include "../Engine/Animation/AnimationComponent.h"
#include "BattleUI/BattleLoopLayer.h"
#include "BattleUI/MoveSelectLayer.h"
#include "BattleUI/OptionSelectLayer.h"
#include "BattleUI/RunAwayLayer.h"
#include "Monsters/MonsterPartyComponent.h"
#include "Monsters/PocketMonsterEntity.h"

BattleState::BattleState(GameContext& gameContext, const BattleBeginContext& battleContext) :
	m_gameContext(gameContext),
	m_isInBattleLoop(false),
	m_currentUILayer(OptionSelect)
{
	m_battleContext = battleContext;

	m_UILayers = std::array<std::unique_ptr<UILayer>, eUILayerType::COUNT>{
		std::make_unique<OptionSelectLayer>(),
		std::make_unique<MoveSelectLayer>(),
		nullptr,
		nullptr,
		std::make_unique<BattleLoopLayer>(),
		std::make_unique<RunAwayLayer>(),
	};

	m_inputMapper.Map(
		SELECT,
		eInputType::Keyboard,
		static_cast<int>(sf::Keyboard::Key::Enter),
		static_cast<int>(sf::Keyboard::Key::Space)
	);
	m_inputMapper.Map(
		UP,
		eInputType::Keyboard,
		static_cast<int>(sf::Keyboard::Key::W),
		static_cast<int>(sf::Keyboard::Key::Up)
	);
	m_inputMapper.Map(
		DOWN,
		eInputType::Keyboard,
		static_cast<int>(sf::Keyboard::Key::S),
		static_cast<int>(sf::Keyboard::Key::Down)
	);
	m_inputMapper.Map(
		LEFT, eInputType::Keyboard,
		static_cast<int>(sf::Keyboard::Key::A),
		static_cast<int>(sf::Keyboard::Key::Left)
	);
	m_inputMapper.Map(
		RIGHT,
		eInputType::Keyboard,
		static_cast<int>(sf::Keyboard::Key::D),
		static_cast<int>(sf::Keyboard::Key::Right)
	);

	m_inputMapper.OnButtonPressed(UP, [this]() { OnNavigateButtonPressed(UP); });
	m_inputMapper.OnButtonPressed(DOWN, [this]() { OnNavigateButtonPressed(DOWN); });
	m_inputMapper.OnButtonPressed(LEFT, [this]() { OnNavigateButtonPressed(LEFT); });
	m_inputMapper.OnButtonPressed(RIGHT, [this]() { OnNavigateButtonPressed(RIGHT); });
	m_inputMapper.OnButtonPressed(SELECT, [this]() { OnSelectButtonPressed(); });

	// SET UP PLAYER ENTITY AND PARTY
	Entity* playerEntity = m_gameContext.m_Entities.Get(battleContext.m_PlayerEntityID);
	ASSERT(playerEntity->HasComponent<MonsterPartyComponent>());
	auto* playerMPC = playerEntity->GetComponent<MonsterPartyComponent>();
	playerMPC->OnBattleBegin();

	PocketMonsterEntity* playerMonster = playerMPC->GetActiveMonster();
	ASSERT_MSG(playerMonster, "Something has gone very wrong!");

	playerMonster->GetComponent<EntityAnimationComponent>()->PlayAnimation(EntityAnimationComponent::BATTLE_BACK, true);

	m_playerMonsterEntityID = playerMonster->GetID();


	// SET UP OPPONENT ENTITY AND PARTY
	Entity* opponentEntity = m_gameContext.m_Entities.Get(battleContext.m_OpponentEntityID);

	// Ensure the opponent components are active!
	if (!opponentEntity->IsActive())
	{
		opponentEntity->OnActivate();
	}

	ASSERT(opponentEntity->HasComponent<MonsterPartyComponent>());
	auto* opponentMPC = opponentEntity->GetComponent<MonsterPartyComponent>();
	opponentMPC->OnBattleBegin();

	PocketMonsterEntity* opponentMonster = opponentMPC->GetActiveMonster();
	ASSERT_MSG(opponentMonster, "Something has gone very wrong!");

	opponentMonster->GetComponent<EntityAnimationComponent>()->PlayAnimation(
		EntityAnimationComponent::BATTLE_FRONT, true);

	m_opponentMonsterEntityID = opponentMonster->GetID();


	// TODO: Dynamically position them based on the sprites
	playerMonster->SetPosition({ 206, 389 });
	opponentMonster->SetPosition({ 600, 236 });


	std::cout << "Battle begun! Player monster: Level " << static_cast<int>(playerMonster->GetLevel()) << " " <<
		StringTable::Get()->GetString(playerMonster->GetNameStringID()) << " with entity ID " << playerMonster->GetID()
		<< "\n";
	playerMonster->GetStats().Log();

	std::cout << "Opponent monster: " << "Level " << static_cast<int>(opponentMonster->GetLevel()) << " " <<
		StringTable::Get()->GetString(opponentMonster->GetNameStringID()) << " with entity ID " << opponentMonster->
		GetID() << "\n";
	opponentMonster->GetStats().Log();
}

void BattleState::OnEnter()
{
	printf("Entered a battle!\n");
	m_battleFinished = false;

	UIMANAGER.GetElement(BATTLE_PANEL_NAME)->OnActivate();
	DialogueBox::SetVisible(false);

	m_UILayers[m_currentUILayer]->OnActivate(*this, UILayer::LayerResult{});

	m_onBattleEndEventID = game_events::OnBattleEnd.On([this](const BattleEndContext&)
	{
			m_battleFinished = true;
		});
}

void BattleState::OnExit()
{
	printf("Battle finished!\n");

	DialogueBox::SetVisible(false);

	UIMANAGER.GetElement(BATTLE_PANEL_NAME)->OnDeactivate();

	m_UILayers[m_currentUILayer]->OnDeactivate();

	Entity* player = m_gameContext.m_Entities.Get<Entity>(m_gameContext.m_PlayerEntityID);
	ASSERT(player && player->HasComponent<MonsterPartyComponent>());

	auto mpc = player->GetComponent<MonsterPartyComponent>();
	mpc->OnBattleEnd();

	m_gameContext.m_Entities.Destroy(m_opponentMonsterEntityID);

	game_events::OnBattleEnd.Off(m_onBattleEndEventID);
}

void BattleState::Update(const float deltaTime)
{
	m_inputMapper.Update();

	if (m_battleFinished)
	{
		return;
	}

	auto* playerMonster = m_gameContext.m_Entities.Get<Entity>(m_playerMonsterEntityID);
	ASSERT(playerMonster != nullptr);
	playerMonster->Update(deltaTime);

	auto* opponentMonster = m_gameContext.m_Entities.Get<Entity>(m_opponentMonsterEntityID);
	ASSERT(opponentMonster != nullptr);
	opponentMonster->Update(deltaTime);

	ASSERT(m_UILayers[m_currentUILayer] != nullptr);

	auto& currentLayer = m_UILayers[m_currentUILayer];
	currentLayer->Update(deltaTime);

	if (currentLayer->IsFinished())
	{
		currentLayer->OnDeactivate();

		const UILayer::LayerResult currentLayerResult = currentLayer->GetLayerResult();

		m_currentUILayer = currentLayerResult.m_NextLayer;

		ASSERT(m_UILayers[m_currentUILayer] != nullptr);
		m_UILayers[m_currentUILayer]->OnActivate(*this, currentLayerResult);
	}
}

void BattleState::Render(sf::RenderWindow& window) const
{
	int8_t i = UiManager::k_bottomLayer;

	// TODO: This HORRENDOUSLY needs a cleaner interface - we could do with a pass on the whole rendering of the engine tbph
	for (; i <= 0; ++i)
	{
		UIMANAGER.RenderLayer(window, i);
	}

	Entity* playerMonster = m_gameContext.m_Entities.Get<Entity>(m_playerMonsterEntityID);
	if (playerMonster != nullptr)
	{
		m_gameContext.m_Renderer.RenderEntity(window, playerMonster);
	}

	Entity* opponentMonster = m_gameContext.m_Entities.Get<Entity>(m_opponentMonsterEntityID);
	if (opponentMonster != nullptr)
	{
		m_gameContext.m_Renderer.RenderEntity(window, opponentMonster);
	}

	for (; i < UiManager::k_topLayer; ++i)
	{
		UIMANAGER.RenderLayer(window, i);
	}

	return;
}

GameContext& BattleState::GetGameContext() const
{
	return m_gameContext;
}

const BattleBeginContext& BattleState::GetBattleContext() const
{
	return m_battleContext;
}

entity_id_t BattleState::GetPlayerMonsterEntityID() const
{
	return m_playerMonsterEntityID;
}

entity_id_t BattleState::GetOpponentMonsterEntityID() const
{
	return m_opponentMonsterEntityID;
}

void BattleState::OnNavigateButtonPressed(const eUILayerNavigateButtons button) const
{
	m_UILayers[m_currentUILayer]->OnNavigateButtonPressed(button);
}

void BattleState::OnSelectButtonPressed() const
{
	m_UILayers[m_currentUILayer]->OnSelectButtonPressed();
}
