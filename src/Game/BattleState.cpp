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
	m_currentUILayer(OptionSelect),
	m_isInBattleLoop(false)
{
	m_battleContext = battleContext;

	m_UILayers = std::array<std::unique_ptr<UILayer>, eUILayer::COUNT>{
		std::make_unique<OptionSelectLayer>(),
		std::make_unique<FightLayer>(),
		nullptr,
		nullptr,
		std::make_unique<BattleLoopLayer>(),
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

		if (m_currentUILayer == MoveSelect && currentLayerResult.m_NextLayer == BattleLoop)
		{
			TriggerBattleLoop(currentLayerResult.m_ChosenMoveIndex.value());
		}

		m_currentUILayer = currentLayerResult.m_NextLayer;

		ASSERT(m_UILayers[m_currentUILayer] != nullptr);
		m_UILayers[m_currentUILayer]->OnActivate(*this, currentLayerResult);
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

void BattleState::UILayer::OnActivate(const BattleState& /*ctx*/, const LayerResult& /*prevLayerResult*/)
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

void BattleState::OptionSelectLayer::OnActivate(const BattleState& state, const LayerResult& prevLayerResult)
{
	UILayer::OnActivate(state, prevLayerResult);

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
	m_validMoves(),
	m_selectedMove(eSelection::Move1)
{
}

BattleState::UILayer::LayerResult BattleState::FightLayer::GetLayerResult() const
{
	return {
		.m_NextLayer = BattleLoop,
		.m_ChosenMoveIndex = static_cast<uint8_t>(m_selectedMove),
	};
}

void BattleState::FightLayer::OnNavigateButtonPressed(const eInputs button)
{
	const bool upDown = button & (UP | DOWN);
	const bool leftRight = button & (LEFT | RIGHT);

	if (upDown)
	{
		switch (m_selectedMove)
		{
		case eSelection::Move1:
			if (m_validMoves[static_cast<size_t>(eSelection::Move3)])
			{
				OnSelectedMoveChanged(eSelection::Move3);
			}
			break;
		case eSelection::Move2:
			if (m_validMoves[static_cast<size_t>(eSelection::Move4)])
			{
				OnSelectedMoveChanged(eSelection::Move4);
			}
			break;
		case eSelection::Move3:
			if (m_validMoves[static_cast<size_t>(eSelection::Move1)])
			{
				OnSelectedMoveChanged(eSelection::Move1);
			}
			break;
		case eSelection::Move4:
			if (m_validMoves[static_cast<size_t>(eSelection::Move2)])
			{
				OnSelectedMoveChanged(eSelection::Move2);
			}
			break;
		}
	}
	else if (leftRight)
	{
		switch (m_selectedMove)
		{
		case eSelection::Move1:
			if (m_validMoves[static_cast<size_t>(eSelection::Move2)])
			{
				OnSelectedMoveChanged(eSelection::Move2);
			}
			break;
		case eSelection::Move2:
			if (m_validMoves[static_cast<size_t>(eSelection::Move1)])
			{
				OnSelectedMoveChanged(eSelection::Move1);
			}
			break;
		case eSelection::Move3:
			if (m_validMoves[static_cast<size_t>(eSelection::Move4)])
			{
				OnSelectedMoveChanged(eSelection::Move4);
			}
			break;
		case eSelection::Move4:
			if (m_validMoves[static_cast<size_t>(eSelection::Move3)])
			{
				OnSelectedMoveChanged(eSelection::Move3);
			}
			break;
		}
	}
}

void BattleState::FightLayer::OnSelectButtonPressed()
{
	ASSERT(this->m_playerMonster != nullptr);

	const MoveComponent* playerMoveComponent = this->m_playerMonster->GetComponent<MoveComponent>();
	ASSERT(playerMoveComponent);

	m_finished = playerMoveComponent->CanUseMove(static_cast<uint8_t>(m_selectedMove));
}

void BattleState::FightLayer::OnActivate(const BattleState& state, const LayerResult& prevLayerResult)
{
	UILayer::OnActivate(state, prevLayerResult);

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

	this->m_opponentMonster = state.m_gameContext.m_Entities.Get<PocketMonsterEntity>(state.m_opponentMonsterEntityID);
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

	OnSelectedMoveChanged(eSelection::Move1);
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

void BattleState::FightLayer::OnSelectedMoveChanged(const eSelection newMove)
{
	m_selectedMove = newMove;

	const auto* battleUI = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battleUI != nullptr);

	const auto* moveSelectUI = dynamic_cast<UiPanel*>(battleUI->GetChild(MOVES_PANEL_NAME));
	ASSERT(moveSelectUI != nullptr);

	auto* move1 = moveSelectUI->GetChild("MOVE_1_ARROW");
	ASSERT(move1 != nullptr);

	auto* move2 = moveSelectUI->GetChild("MOVE_2_ARROW");
	ASSERT(move2 != nullptr);

	auto* move3 = moveSelectUI->GetChild("MOVE_3_ARROW");
	ASSERT(move3 != nullptr);

	auto* move4 = moveSelectUI->GetChild("MOVE_4_ARROW");
	ASSERT(move4 != nullptr);

	move1->OnDeactivate();
	move2->OnDeactivate();
	move3->OnDeactivate();
	move4->OnDeactivate();

	switch (newMove)
	{
	case eSelection::Move1:
		move1->OnActivate();
		break;
	case eSelection::Move2:
		move2->OnActivate();
		break;
	case eSelection::Move3:
		move3->OnActivate();
		break;
	case eSelection::Move4:
		move4->OnActivate();
		break;
	}
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

void BattleState::RunLayer::OnActivate(const BattleState& state, const LayerResult& prevLayerResult)
{
	UILayer::OnActivate(state, prevLayerResult);

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

BattleState::BattleLoopLayer::BattleLoopLayer() :
	m_playerMonster(nullptr),
	m_opponentMonster(nullptr),
	m_playerChosenMoveIdx(0xFF),
	m_currentState(eBattleFlow::PlayerSwitch)
{
}

BattleState::UILayer::LayerResult BattleState::BattleLoopLayer::GetLayerResult() const
{
	return { .m_NextLayer = OptionSelect };
}

void BattleState::BattleLoopLayer::OnSelectButtonPressed()
{
	switch (m_currentState)
	{
	case eBattleFlow::PlayerSwitch:
		TransitionToFlowState(eBattleFlow::PlayerMoveName);
		break;
	case eBattleFlow::OpponentSwitch:
		TransitionToFlowState(eBattleFlow::PlayerMoveName);
		break;
	case eBattleFlow::PlayerMoveName:
		TransitionToFlowState(eBattleFlow::PlayerMoveEffectiveness);
		break;
	case eBattleFlow::PlayerMoveEffectiveness:
		TransitionToFlowState(eBattleFlow::OpponentFaintOrSwitch);
		break;
	case eBattleFlow::OpponentFaintOrSwitch:
		TransitionToFlowState(eBattleFlow::OpponentMoveName);
		break;
	case eBattleFlow::OpponentMoveName:
		TransitionToFlowState(eBattleFlow::OpponentMoveEffectiveness);
		break;
	case eBattleFlow::OpponentMoveEffectiveness:
		TransitionToFlowState(eBattleFlow::PlayerFaint);
		break;
	case eBattleFlow::PlayerFaint:
		m_finished = true;
		break;
	}
}

void BattleState::BattleLoopLayer::OnActivate(const BattleState& state, const LayerResult& prevLayerResult)
{
	UILayer::OnActivate(state, prevLayerResult);

	ASSERT(prevLayerResult.m_ChosenMoveIndex.has_value());
	this->m_playerChosenMoveIdx = prevLayerResult.m_ChosenMoveIndex.value();

	this->m_playerMonster = state.m_gameContext.m_Entities.Get<PocketMonsterEntity>(state.m_playerMonsterEntityID);
	ASSERT(this->m_playerMonster != nullptr);

	this->m_opponentMonster = state.m_gameContext.m_Entities.Get<PocketMonsterEntity>(state.m_opponentMonsterEntityID);
	ASSERT(this->m_opponentMonster != nullptr);

	bool hasPlayerSwitched = false;
	bool hasOpponentSwitched = false;

	if (hasPlayerSwitched)
	{
		TransitionToFlowState(eBattleFlow::PlayerSwitch);
	}
	else if (hasOpponentSwitched)
	{
		TransitionToFlowState(eBattleFlow::OpponentSwitch);
	}
	else
	{
		TransitionToFlowState(eBattleFlow::PlayerMoveName);
	}
}

void BattleState::BattleLoopLayer::OnDeactivate()
{
	UILayer::OnDeactivate();
	DialogueBox::SetVisible(false);
}

void BattleState::BattleLoopLayer::OnNavigateButtonPressed(eInputs button)
{
}

void BattleState::BattleLoopLayer::TransitionToFlowState(const eBattleFlow state)
{
#if BUILD_DEBUG
	std::cout << "Transitioning to state " << magic_enum::enum_name(state) << "\n";
#endif

	DialogueBox::SetVisible(true);

	switch (state)
	{
	case eBattleFlow::PlayerSwitch:
		break;
	case eBattleFlow::OpponentSwitch:
		break;
	case eBattleFlow::PlayerMoveName:
		{
			const std::string monsterName = STRINGTABLE->GetString(
				"MONSTER_NAME",
				m_playerMonster->GetNameStringID()
			);

			const std::string moveName = m_playerMonster->GetMoveName(m_playerChosenMoveIdx);

			const std::string text = STRINGTABLE->GetDynamicString(
				HASH("PLAYER_MOVE_NAME"),
				monsterName,
				moveName
			);

			DialogueBox::SetText(text.c_str());
		}
		break;
	case eBattleFlow::PlayerMoveEffectiveness:
		break;
	case eBattleFlow::OpponentFaintOrSwitch:
		break;
	case eBattleFlow::OpponentMoveName:
		break;
	case eBattleFlow::OpponentMoveEffectiveness:
		break;
	case eBattleFlow::PlayerFaint:
		break;
	}

	m_currentState = state;
}


void BattleState::OnNavigateButtonPressed(const eInputs button) const
{
	m_UILayers[m_currentUILayer]->OnNavigateButtonPressed(button);
}

void BattleState::OnSelectButtonPressed() const
{
	m_UILayers[m_currentUILayer]->OnSelectButtonPressed();
}

void BattleState::TriggerBattleLoop(const uint8_t chosenPlayerMoveIdx)
{
	ASSERT(chosenPlayerMoveIdx < MOVE_COUNT);

	PocketMonsterEntity* playerMonster = m_gameContext.m_Entities.Get<PocketMonsterEntity>(m_playerMonsterEntityID);
	PocketMonsterEntity* opponentMonster = m_gameContext.m_Entities.Get<PocketMonsterEntity>(m_opponentMonsterEntityID);

	ASSERT(playerMonster != nullptr);
	ASSERT(opponentMonster != nullptr);

	auto* playerMoveComponent = playerMonster->GetComponent<MoveComponent>();
	ASSERT(playerMoveComponent);
	if (playerMoveComponent->CanUseMove(chosenPlayerMoveIdx))
	{
		playerMoveComponent->UseMove(chosenPlayerMoveIdx, *opponentMonster);
	}

	auto* opponentMoveComponent = opponentMonster->GetComponent<MoveComponent>();
	ASSERT(opponentMoveComponent);
	if (opponentMoveComponent->CanUseMove(chosenPlayerMoveIdx))
	{
		opponentMoveComponent->UseMove(chosenPlayerMoveIdx, *playerMonster);
	}
}
