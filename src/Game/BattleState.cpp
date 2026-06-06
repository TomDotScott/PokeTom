#include "BattleState.h"

#include "DialogueBox.h"
#include "GameEvents.h"
#include "MoveComponent.h"
#include "../Engine/Animation/AnimationComponent.h"
#include "Monsters/PocketMonsterEntity.h"
#include "../Engine/Stringtable.h"

constexpr static auto BATTLE_PANEL_NAME = "BATTLE_HUD_PANEL";
constexpr static auto OPTIONS_PANEL_NAME = "BATTLE_OPTIONS";

BattleState::BattleState(GameContext& gameContext, BattleBeginContext battleContext):
	m_gameContext(gameContext),
	m_battleContext(std::move(battleContext))
{
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
	m_inputMapper.Map(
		ONE,
		eInputType::Keyboard,
		static_cast<int>(sf::Keyboard::Key::Num1)
	);
	m_inputMapper.Map(
		TWO,
		eInputType::Keyboard,
		static_cast<int>(sf::Keyboard::Key::Num2)
	);
	m_inputMapper.Map(
		THREE,
		eInputType::Keyboard,
		static_cast<int>(sf::Keyboard::Key::Num3)
	);
	m_inputMapper.Map(
		FOUR,
		eInputType::Keyboard,
		static_cast<int>(sf::Keyboard::Key::Num4)
	);

	m_inputMapper.OnButtonPressed(UP, [this]() { OnNavigateButtonPressed(UP); });
	m_inputMapper.OnButtonPressed(DOWN, [this]() { OnNavigateButtonPressed(DOWN); });
	m_inputMapper.OnButtonPressed(LEFT, [this]() { OnNavigateButtonPressed(LEFT); });
	m_inputMapper.OnButtonPressed(RIGHT, [this]() { OnNavigateButtonPressed(RIGHT); });

	// TODO: Find the first Monster in the party that has HP
	// TODO: This might not even have to be part of the battlecontext if I make it a component attached to the player, we have the player's entity ID here so we can get it through that
	m_playerMonsterEntityID = m_battleContext.m_PlayerMonsterEntityIDs[0];
	auto playerMonster = gameContext.m_Entities.Get<PocketMonsterEntity>(m_playerMonsterEntityID);
	playerMonster->OnActivate();

	PocketMonsterEntity& opponentMonster = gameContext.m_Entities.Create<PocketMonsterEntity>(
		m_battleContext.m_opponentMonsters[0].GetID(),
		m_battleContext.m_opponentMonsterLevels[0],
		EntityAnimationComponent::eAnimationName::BATTLE_FRONT
	);

	m_opponentMonsterEntityID = opponentMonster.GetID();

	// TODO: Dynamically position them based on the sprites
	playerMonster->SetPosition({ 206, 389 });
	opponentMonster.SetPosition({ 600, 236 });


	std::cout << "Battle begun! Player monster: Level " << static_cast<int>(playerMonster->GetLevel()) << " " << StringTable::Get()->GetString(playerMonster->GetNameStringID()) << " with entity ID " << playerMonster->GetID() << "\n";
	playerMonster->GetStats().Log();

	std::cout << "Opponent monster: " << "Level " << static_cast<int>(opponentMonster.GetLevel()) << " " << StringTable::Get()->GetString(opponentMonster.GetNameStringID()) << " with entity ID " << opponentMonster.GetID() << "\n";
	opponentMonster.GetStats().Log();
}

void BattleState::OnEnter()
{
	printf("Entered a battle!\n");

	UIMANAGER.GetElement(BATTLE_PANEL_NAME)->OnActivate();
	DialogueBox::SetVisible(false);

	OnSelectedOptionChanged(eSelectedOption::FIGHT);
}

void BattleState::OnExit()
{
	printf("Battle finished!\n");
	UIMANAGER.GetElement(BATTLE_PANEL_NAME)->OnDeactivate();

	//m_gameContext.m_Entities.Destroy(m_playerMonsterEntityID);
	for (auto id : m_battleContext.m_PlayerMonsterEntityIDs)
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

	// TODO: When this is moved to an event, BattleState::Update throws an exception as the entities have been deleted... I need to investigate properly why this is (I think something to do with "this" being captured in the lambda group means the object lifetime is extended past where it needs to be?). Otherwise, we can't run away from battles in the button pressed event fire!
	if (m_inputMapper.IsButtonPressed(SELECT))
	{
		game_events::OnBattleEnd.Fire({
			.m_LevelHash = m_battleContext.m_LevelHash,
			.m_PlayerPosition = m_battleContext.m_PlayerPosition,
		});
	}

	if (m_inputMapper.IsButtonPressed(ONE))
	{
		playerMonster->GetComponent<MoveComponent>()->UseMove(0, *opponentMonster);
	}else if (m_inputMapper.IsButtonPressed(TWO))
	{
		playerMonster->GetComponent<MoveComponent>()->UseMove(1, *opponentMonster);

	}
}

void BattleState::Render(sf::RenderWindow& window) const
{
	UIMANAGER.RenderBackground(window);

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

	UIMANAGER.RenderMidground(window);
	UIMANAGER.RenderForeground(window);
}

void BattleState::OnNavigateButtonPressed(const eInputs button)
{
	// FIGHT       BAG
	// POKEMON     RUN
	const bool upDown = button & (UP | DOWN);
	const bool leftRight = button & (LEFT | RIGHT);
	if (upDown)
	{
		switch (m_selectedOption)
		{
		case eSelectedOption::FIGHT:
			OnSelectedOptionChanged(eSelectedOption::MONSTERS);
			break;
		case eSelectedOption::MONSTERS:
			OnSelectedOptionChanged(eSelectedOption::FIGHT);
			break;
		case eSelectedOption::BAG:
			OnSelectedOptionChanged(eSelectedOption::RUN);
			break;
		case eSelectedOption::RUN:
			OnSelectedOptionChanged(eSelectedOption::BAG);
			break;
		}
	}
	else if (leftRight)
	{
		switch (m_selectedOption)
		{
		case eSelectedOption::FIGHT:
			OnSelectedOptionChanged(eSelectedOption::BAG);
			break;
		case eSelectedOption::MONSTERS:
			OnSelectedOptionChanged(eSelectedOption::RUN);
			break;
		case eSelectedOption::BAG:
			OnSelectedOptionChanged(eSelectedOption::FIGHT);
			break;
		case eSelectedOption::RUN:
			OnSelectedOptionChanged(eSelectedOption::MONSTERS);
			break;
		}
	}
}

void BattleState::OnSelectedOptionChanged(const eSelectedOption newOption)
{
	m_selectedOption = newOption;

	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	auto* optionsUI = dynamic_cast<UiPanel*>(battleUI->GetChild(OPTIONS_PANEL_NAME));
	ASSERT(optionsUI != nullptr);

	auto* fightArrow = optionsUI->GetChild("FIGHT_ARROW");
	ASSERT(fightArrow != nullptr);

	auto* monstersArrow = optionsUI->GetChild("MONSTERS_ARROW");
	ASSERT(monstersArrow != nullptr);

	auto* bagArrow = optionsUI->GetChild("BAG_ARROW");
	ASSERT(bagArrow != nullptr);

	auto* runArrow = optionsUI->GetChild("RUN_ARROW");
	ASSERT(runArrow != nullptr);


	fightArrow->OnDeactivate();
	monstersArrow->OnDeactivate();
	bagArrow->OnDeactivate();
	runArrow->OnDeactivate();

	switch (newOption)
	{
	case eSelectedOption::FIGHT:
		fightArrow->OnActivate();
		break;
	case eSelectedOption::MONSTERS:
		monstersArrow->OnActivate();
		break;
	case eSelectedOption::BAG:
		bagArrow->OnActivate();
		break;
	case eSelectedOption::RUN:
		runArrow->OnActivate();
		break;
	}
}
