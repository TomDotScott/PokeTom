#include "BattleState.h"

#include "DialogueBox.h"
#include "GameEvents.h"
#include "MoveComponent.h"
#include "../Engine/Animation/AnimationComponent.h"
#include "Monsters/PocketMonsterEntity.h"
#include "../Engine/Stringtable.h"

constexpr static auto BATTLE_PANEL_NAME = "BATTLE_HUD_PANEL";
constexpr static auto OPTIONS_PANEL_NAME = "BATTLE_OPTIONS";
constexpr static auto MOVES_PANEL_NAME = "MOVE_OPTIONS";

BattleState::BattleState(GameContext& gameContext, const BattleBeginContext& battleContext):
	m_gameContext(gameContext),
	m_currentUILayer(OptionSelect)
{
	m_battleContext = battleContext;

	m_UILayers = std::array<std::unique_ptr<UILayer>, eUILayer::COUNT>{
		std::make_unique<OptionSelectLayer>(),
		std::make_unique<FightLayer>(),
		nullptr,
		nullptr,
		std::make_unique<RunLayer>(),
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


	std::cout << "Battle begun! Player monster: Level " << static_cast<int>(playerMonster->GetLevel()) << " " <<
		StringTable::Get()->GetString(playerMonster->GetNameStringID()) << " with entity ID " << playerMonster->GetID()
		<< "\n";
	playerMonster->GetStats().Log();

	std::cout << "Opponent monster: " << "Level " << static_cast<int>(opponentMonster.GetLevel()) << " " <<
		StringTable::Get()->GetString(opponentMonster.GetNameStringID()) << " with entity ID " << opponentMonster.
		GetID() << "\n";
	opponentMonster.GetStats().Log();
}

void BattleState::OnEnter()
{
	printf("Entered a battle!\n");

	UIMANAGER.GetElement(BATTLE_PANEL_NAME)->OnActivate();
	DialogueBox::SetVisible(false);

	m_UILayers[m_currentUILayer]->OnActivate(*this);
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
		m_UILayers[m_currentUILayer]->OnActivate(*this);
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

BattleState::UILayer::UILayer(): m_finished(false)
{
}

bool BattleState::UILayer::IsFinished() const
{
	return m_finished;
}

void BattleState::UILayer::Update(float /*deltaTime*/)
{
}

void BattleState::UILayer::OnActivate(const BattleState& /*ctx*/)
{
	m_finished = false;
}

void BattleState::UILayer::OnDeactivate()
{
}

BattleState::OptionSelectLayer::OptionSelectLayer() :
	UILayer(),
	m_selectedOption()
{
}

BattleState::UILayer::LayerResult BattleState::OptionSelectLayer::GetLayerResult() const
{
	switch (m_selectedOption)
	{
	case eSelectedOption::Fight:
		return { .m_NextLayer = MoveSelect };
	case eSelectedOption::Monsters:
		return { .m_NextLayer = MonsterSelect };
	case eSelectedOption::Bag:
		return { .m_NextLayer = ItemSelect };
	case eSelectedOption::Run:
		return { .m_NextLayer = QuitBattle };
	}

	return { .m_NextLayer = OptionSelect };
}

void BattleState::OptionSelectLayer::OnNavigateButtonPressed(const eInputs button)
{
	const bool upDown = button & (UP | DOWN);
	const bool leftRight = button & (LEFT | RIGHT);

	// FIGHT       BAG
	// POKEMON     RUN
	if (upDown)
	{
		switch (m_selectedOption)
		{
		case eSelectedOption::Fight:
			OnSelectedOptionChanged(eSelectedOption::Monsters);
			break;
		case eSelectedOption::Monsters:
			OnSelectedOptionChanged(eSelectedOption::Fight);
			break;
		case eSelectedOption::Bag:
			OnSelectedOptionChanged(eSelectedOption::Run);
			break;
		case eSelectedOption::Run:
			OnSelectedOptionChanged(eSelectedOption::Bag);
			break;
		}
	}
	else if (leftRight)
	{
		switch (m_selectedOption)
		{
		case eSelectedOption::Fight:
			OnSelectedOptionChanged(eSelectedOption::Bag);
			break;
		case eSelectedOption::Monsters:
			OnSelectedOptionChanged(eSelectedOption::Run);
			break;
		case eSelectedOption::Bag:
			OnSelectedOptionChanged(eSelectedOption::Fight);
			break;
		case eSelectedOption::Run:
			OnSelectedOptionChanged(eSelectedOption::Monsters);
			break;
		}
	}
}

void BattleState::OptionSelectLayer::OnSelectButtonPressed()
{
	m_finished = true;
}

void BattleState::OptionSelectLayer::OnActivate(const BattleState& state)
{
	UILayer::OnActivate(state);

	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	auto* optionsUI = dynamic_cast<UiPanel*>(battleUI->GetChild(OPTIONS_PANEL_NAME));
	ASSERT(optionsUI != nullptr);

	auto* moveUI = dynamic_cast<UiPanel*>(battleUI->GetChild(MOVES_PANEL_NAME));
	ASSERT(moveUI != nullptr);

	optionsUI->OnActivate();
	moveUI->OnDeactivate();

	OnSelectedOptionChanged(eSelectedOption::Fight);
}

void BattleState::OptionSelectLayer::OnDeactivate()
{
	UILayer::OnDeactivate();

	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	auto* optionsUI = dynamic_cast<UiPanel*>(battleUI->GetChild(OPTIONS_PANEL_NAME));
	ASSERT(optionsUI != nullptr);

	optionsUI->OnDeactivate();
}

void BattleState::OptionSelectLayer::OnSelectedOptionChanged(const eSelectedOption newOption)
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
	case eSelectedOption::Fight:
		fightArrow->OnActivate();
		break;
	case eSelectedOption::Monsters:
		monstersArrow->OnActivate();
		break;
	case eSelectedOption::Bag:
		bagArrow->OnActivate();
		break;
	case eSelectedOption::Run:
		runArrow->OnActivate();
		break;
	}
}

BattleState::FightLayer::FightLayer() :
	m_playerMonster(nullptr),
	m_opponentMonster(nullptr),
	m_validMoves()
{
}

BattleState::UILayer::LayerResult BattleState::FightLayer::GetLayerResult() const
{
	return { .m_NextLayer = OptionSelect, .m_ChosenMoveID = 41 };
}

void BattleState::FightLayer::OnNavigateButtonPressed(eInputs button)
{
}

void BattleState::FightLayer::OnSelectButtonPressed()
{
	m_finished = true;
}

void BattleState::FightLayer::OnActivate(const BattleState& state)
{
	UILayer::OnActivate(state);

	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	battleUI->GetChild("TEXT_BOX")->OnDeactivate();

	auto* optionsUI = dynamic_cast<UiPanel*>(battleUI->GetChild(OPTIONS_PANEL_NAME));
	ASSERT(optionsUI != nullptr);

	auto* moveUI = dynamic_cast<UiPanel*>(battleUI->GetChild(MOVES_PANEL_NAME));
	ASSERT(moveUI != nullptr);

	optionsUI->OnDeactivate();
	moveUI->OnActivate();


	this->m_playerMonster = state.m_gameContext.m_Entities.Get<PocketMonsterEntity>(state.m_playerMonsterEntityID);
	ASSERT(this->m_playerMonster != nullptr);

	this->m_opponentMonster = state.m_gameContext.m_Entities.Get<PocketMonsterEntity>(state.m_playerMonsterEntityID);
	ASSERT(this->m_opponentMonster != nullptr);

	// Set up the move text
	MoveComponent* moveComponent = this->m_playerMonster->GetComponent<MoveComponent>();
	ASSERT(moveComponent != nullptr);

	static constexpr std::array<const char*, MOVE_COUNT> uiComponentNames{
		"MOVE_1_TEXT",
		"MOVE_2_TEXT",
		"MOVE_3_TEXT",
		"MOVE_4_TEXT",
	};

	for (size_t i = 0; i < MOVE_COUNT; ++i)
	{
		const Move& move = moveComponent->GetMove(i);

		bool isValid = move.IsValid();
		m_validMoves[i] = isValid;

		UiText* moveText = dynamic_cast<UiText*>(moveUI->GetChild(uiComponentNames[i]));
		ASSERT(moveText != nullptr);

		if (isValid)
		{
			moveText->SetText(move.GetNameStringTableID());
		}
		else
		{
			moveText->SetText(" ");
		}
	}
}

void BattleState::FightLayer::OnDeactivate()
{
	UILayer::OnDeactivate();

	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	battleUI->GetChild("TEXT_BOX")->OnActivate();

	auto* moveUI = dynamic_cast<UiPanel*>(battleUI->GetChild(MOVES_PANEL_NAME));
	ASSERT(moveUI != nullptr);

	moveUI->OnDeactivate();
}

BattleState::RunLayer::RunLayer()
{
}

BattleState::UILayer::LayerResult BattleState::RunLayer::GetLayerResult() const
{
	return { .m_NextLayer = OptionSelect };
}

void BattleState::RunLayer::OnNavigateButtonPressed(eInputs /*button*/)
{
}

void BattleState::RunLayer::OnSelectButtonPressed()
{
	game_events::OnBattleEnd.Fire(m_endContext);
}

void BattleState::RunLayer::OnActivate(const BattleState& state)
{
	UILayer::OnActivate(state);

	m_endContext = {
		.m_LevelHash = state.m_battleContext.m_LevelHash,
		.m_PlayerPosition = state.m_battleContext.m_PlayerPosition,
	};

	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	auto* optionsUI = dynamic_cast<UiPanel*>(battleUI->GetChild(OPTIONS_PANEL_NAME));
	ASSERT(optionsUI != nullptr);

	optionsUI->OnDeactivate();

	DialogueBox::SetText("%s got away safely!\n",
	                     STRINGTABLE->GetString(HASH("CHARACTER"), HASH("PLAYER_NAME")).c_str());
	DialogueBox::SetVisible(true);
}

void BattleState::RunLayer::OnDeactivate()
{
	UILayer::OnDeactivate();

	DialogueBox::SetVisible(false);
	auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	battleUI->OnDeactivate();
}

void BattleState::OnNavigateButtonPressed(const eInputs button) const
{
	m_UILayers[m_currentUILayer]->OnNavigateButtonPressed(button);
}

void BattleState::OnSelectButtonPressed() const
{
	m_UILayers[m_currentUILayer]->OnSelectButtonPressed();
}
