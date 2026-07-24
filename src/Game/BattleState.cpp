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

	// TODO: Find the first Monster in the parties that have HP
	// TODO: This might not even have to be part of the battlecontext if I make it a component attached to the player, we have the player's entity ID here so we can get it through that
	m_playerMonsterEntityID = m_battleContext.m_PlayerMonsterEntityIDs[0];
	PocketMonsterEntity* playerMonster = gameContext.m_Entities.Get<PocketMonsterEntity>(m_playerMonsterEntityID);
	playerMonster->OnActivate();

	PocketMonsterEntity* opponentMonster = gameContext.m_Entities.Get<PocketMonsterEntity>(battleContext.m_opponentMonsterEntityIDs[0]);
	ASSERT_MSG(opponentMonster, "Something has gone very wrong!");

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

	UIMANAGER.GetElement(BATTLE_PANEL_NAME)->OnActivate();
	DialogueBox::SetVisible(false);

	m_UILayers[m_currentUILayer]->OnActivate(*this, UILayer::LayerResult{});
}

void BattleState::OnExit()
{
	printf("Battle finished!\n");

	DialogueBox::SetVisible(false);

	UIMANAGER.GetElement(BATTLE_PANEL_NAME)->OnDeactivate();

	m_UILayers[m_currentUILayer]->OnDeactivate();

	for (const auto id : m_battleContext.m_PlayerMonsterEntityIDs)
	{
		auto* e = m_gameContext.m_Entities.Get<Entity>(id);
		e->OnDeactivate();
	}

	m_gameContext.m_Entities.Destroy(m_opponentMonsterEntityID);
}

void BattleState::Update(const float deltaTime)
{
	m_inputMapper.Update();

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
