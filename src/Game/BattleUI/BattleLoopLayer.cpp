#include "BattleLoopLayer.h"

#include <iostream>
#include <frozen/unordered_map.h>

#include "HealthbarAnimation.h"
#include "../BattleState.h"
#include "../DialogueBox.h"
#include "../GameEvents.h"
#include "../../Engine/Maths.h"
#include "../../Engine/Stringtable.h"
#include "../../Engine/UI/UiProgressBar.h"
#include "../Monsters/MonsterPartyComponent.h"
#include "../Monsters/PocketMonsterEntity.h"

namespace
{
	const hash_type MONSTER_NAME_GROUP = HASH("MONSTER_NAME");
}

BattleLoopLayer::BattleLoopLayer() :
	m_playerSwitchedOut(false),
	m_opponentSwitchedOut(false),
	m_playerHealthAnimation(HealthbarAnimation::eAnimationType::Player),
	m_playerExperienceBar(UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME)->GetChild<UiProgressBar>("PLAYER_XP_BAR")),
	m_opponentHealthAnimation(HealthbarAnimation::eAnimationType::Opponent),
	m_endContext()
{
}

UILayer::LayerResult BattleLoopLayer::GetLayerResult() const
{
	return { .m_NextLayer = OptionSelect };
}

void BattleLoopLayer::Update(const float deltaTime)
{
	UILayer::Update(deltaTime);

	m_playerHealthAnimation.Update(deltaTime);
	m_playerExperienceBar.Update(deltaTime);

	m_opponentHealthAnimation.Update(deltaTime);

	if (!m_battleBeatQueue.empty())
	{
		if (auto* anim = std::get_if<AnimationBeat>(&m_battleBeatQueue.front()))
		{
			if (anim->m_IsComplete())
			{
				m_battleBeatQueue.pop();
				AdvanceBeat();
			}
		}
	}
}

void BattleLoopLayer::OnActivate(const BattleState& state, const LayerResult& prevLayerResult)
{
	UILayer::OnActivate(state, prevLayerResult);

	DialogueBox::SetVisible(true);

	const EntityRegistry& entities = state.GetGameContext().m_Entities;

	PocketMonsterEntity* playerMonster = entities.Get<PocketMonsterEntity>(state.GetPlayerMonsterEntityID());
	ASSERT(playerMonster != nullptr);

	m_playerHealthAnimation.SetMonster(playerMonster);

	PocketMonsterEntity* opponentMonster = entities.Get<PocketMonsterEntity>(state.GetOpponentMonsterEntityID());
	ASSERT(opponentMonster != nullptr);

	m_opponentHealthAnimation.SetMonster(opponentMonster);

	m_endContext = BattleEndContext{
		.m_LevelHash = state.GetBattleContext().m_LevelHash,
		.m_PlayerPosition = state.GetBattleContext().m_PlayerPosition,
		.m_SendPlayerToHospital = false
	};

	// TODO:
	m_playerSwitchedOut = false;
	m_opponentSwitchedOut = false;

	ASSERT(prevLayerResult.m_ChosenMoveIndex.has_value());

	// Determine who is first or second
	uint8_t firstMonsterMoveIdx;
	uint8_t secondMonsterMoveIdx;

	// TODO: Properly choose opponent move rather than always 0!
	PocketMonsterEntity* first;
	PocketMonsterEntity* second;

	if (playerMonster->GetStats().m_Speed > opponentMonster->GetStats().m_Speed)
	{
		first = playerMonster;
		firstMonsterMoveIdx = prevLayerResult.m_ChosenMoveIndex.value();

		second = opponentMonster;
		secondMonsterMoveIdx = 0;
	}
	else
	{
		first = opponentMonster;
		firstMonsterMoveIdx = 0;

		second = playerMonster;
		secondMonsterMoveIdx = prevLayerResult.m_ChosenMoveIndex.value();
	}


	ASSERT(first != nullptr);
	ASSERT(second != nullptr);

	const entity_id_t playerMonsterEntityID = playerMonster->GetID();

	DoTurn(state, first, firstMonsterMoveIdx, second, playerMonsterEntityID);
	DoTurn(state, second, secondMonsterMoveIdx, first, playerMonsterEntityID);


	AdvanceBeat();
}

void BattleLoopLayer::OnDeactivate()
{
	UILayer::OnDeactivate();
	DialogueBox::SetVisible(false);
}

void BattleLoopLayer::OnNavigateButtonPressed(eUILayerNavigateButtons /*button*/)
{
}

void BattleLoopLayer::OnSelectButtonPressed()
{
	if (m_battleBeatQueue.empty())
	{
		return;
	}

	if (std::holds_alternative<TextBeat>(m_battleBeatQueue.front()))
	{
		auto beat = std::move(std::get<TextBeat>(m_battleBeatQueue.front()));
		m_battleBeatQueue.pop();

		if (beat.m_OnDismiss.has_value())
		{
			beat.m_OnDismiss.value()();
		}

		AdvanceBeat();
	}
	else if (std::holds_alternative<AnimationBeat>(m_battleBeatQueue.front()))
	{
		auto beat = std::move(std::get<AnimationBeat>(m_battleBeatQueue.front()));
		m_battleBeatQueue.pop();

		if (!beat.m_IsComplete())
		{
			beat.m_FinishAnimation();
		}

		AdvanceBeat();
	}
}

void BattleLoopLayer::OnBackButtonPressed()
{
}

void BattleLoopLayer::DoTurn(
	const BattleState& state,
	PocketMonsterEntity* attacker,
	uint8_t selectedMoveIdx,
	PocketMonsterEntity* defender,
	const entity_id_t playerMonsterEntityID
)
{
	if (attacker->IsFainted())
	{
		return;
	}

	// TODO: Recoil and other effects
	const monster_hp_t attackerHealthBefore = attacker->GetStats().m_HP;
	const monster_hp_t defenderHealthBefore = defender->GetStats().m_HP;

	// Perform the move and then queue the UI messages...
	const Move::Outcome outcome = UseMove(attacker, defender, selectedMoveIdx);

	const monster_hp_t attackerHealthAfter = attacker->GetStats().m_HP;
	const monster_hp_t defenderHealthAfter = defender->GetStats().m_HP;

	// X used Y...
	m_battleBeatQueue.emplace(TextBeat{
		.m_BeatName = "MOVE NAME",
		.m_OnShow = [this, attacker, selectedMoveIdx]()
		{
			ShowMoveNameText(attacker, selectedMoveIdx);
		}
	});

	// The move hits or misses...
	if (outcome.m_MoveMissed)
	{
		m_battleBeatQueue.emplace(TextBeat{
			.m_BeatName = "THE MOVE MISSED!",
			.m_OnShow = [this, attacker]()
			{
				ShowMissText(attacker);
			}
		});

		return;
	}

	if (defenderHealthBefore - defenderHealthAfter != 0)
	{
		if (defender->GetID() == playerMonsterEntityID)
		{
			m_battleBeatQueue.emplace(AnimationBeat{
				.m_BeatName = "PLAYER_HEALTH_BAR",
				.m_Start = [this, defenderHealthBefore]
				{
					m_playerHealthAnimation.Start(defenderHealthBefore);
				},
				.m_IsComplete = [this] { return !m_playerHealthAnimation.IsPlaying(); },
				.m_FinishAnimation = [this] { m_playerHealthAnimation.Finish(); }
			});
		}
		else
		{
			m_battleBeatQueue.emplace(AnimationBeat{
				.m_BeatName = "OPP_HEALTH_BAR",
				.m_Start = [this, defenderHealthBefore]
				{
					m_opponentHealthAnimation.Start(defenderHealthBefore);
				},
				.m_IsComplete = [this] { return !m_opponentHealthAnimation.IsPlaying(); },
				.m_FinishAnimation = [this] { m_opponentHealthAnimation.Finish(); }
			});
		}
	}

	// Effectiveness message...
	const float effectiveness = outcome.m_TypeMultiplier;
	if (effectiveness != 1.0f)
	{
		m_battleBeatQueue.emplace(TextBeat{
			.m_BeatName = "IMMUNE/VERY/NOT VERY EFFECTIVE TEXT",
			.m_OnShow = [this, effectiveness]()
			{
				ShowEffectivenessText(effectiveness);
			}
		});
	}

	// Was it a crit?
	if (outcome.m_IsCriticalHit)
	{
		m_battleBeatQueue.emplace(TextBeat{
			.m_BeatName = "CRITICAL HIT!",
			.m_OnShow = [this]()
			{
				ShowCriticalHitText();
			}
		});
	}

	// Any Stat Changes
	if (outcome.m_StatChangeOutcome.has_value())
	{
		const auto& [attackerStatChanges, defenderStatChanges] = outcome.m_StatChangeOutcome.value();

		// TODO: Power up/down animation on the sprites - maybe a shader?
		for (const auto& statChanges : defenderStatChanges)
		{
			// TODO: Queue AnimationBeat
			m_battleBeatQueue.emplace(TextBeat{
				.m_BeatName = "DEFENDER STAT CHANGE",
				.m_OnShow = [this, defender, statChanges]()
				{
					ShowStatChangeText(defender, statChanges.m_Stage, statChanges.m_Succeeded);
				}
			});
		}

		for (const auto& statChanges : attackerStatChanges)
		{
			// TODO: Queue AnimationBeat
			m_battleBeatQueue.emplace(TextBeat{
				.m_BeatName = "ATTACKER STAT CHANGE",
				.m_OnShow = [this, attacker, statChanges]()
				{
					ShowStatChangeText(attacker, statChanges.m_Stage, statChanges.m_Succeeded);
				}
			});
		}
	}

	// Any Status Conditions
	// Recoil or Health Drain/Gain?
	// Faint Check
	if (attacker->IsFainted())
	{
		OnMonsterFainted(
			state,
			attacker,
			attacker->GetID() == playerMonsterEntityID
		);
	}

	if (defender->IsFainted())
	{
		OnMonsterFainted(
			state,
			defender,
			defender->GetID() == playerMonsterEntityID
		);
	}
}


void BattleLoopLayer::AdvanceBeat()
{
	if (m_battleBeatQueue.empty())
	{
		m_finished = true;
		return;
	}

	std::visit(overloaded{
		           [](const TextBeat& beat)
		           {
			           if (beat.m_OnShow)
			           {
				           beat.m_OnShow();
				           // Waits for the A press in OnSelectButtonPressed
			           }
		           },
		           [](const AnimationBeat& beat)
		           {
			           if (beat.m_Start)
			           {
				           beat.m_Start();
			           }
		           }
	           }, m_battleBeatQueue.front());
}

void BattleLoopLayer::ShowMoveNameText(const PocketMonsterEntity* monster, const uint8_t moveIdx) const
{
	// TODO: Nicknames
	const std::string monsterName = STRINGTABLE->GetString(MONSTER_NAME_GROUP, monster->GetNameStringID());
	const std::string moveName = monster->GetMoveName(moveIdx);
	DialogueBox::SetText(STRINGTABLE->GetDynamicString(HASH("MONSTER_MOVE_NAME"), monsterName, moveName).c_str());
}

void BattleLoopLayer::ShowFaintText(const PocketMonsterEntity* monster) const
{
	// TODO: Nicknames
	const std::string monsterName = STRINGTABLE->GetString(MONSTER_NAME_GROUP, monster->GetNameStringID());
	DialogueBox::SetText(STRINGTABLE->GetDynamicString(HASH("MONSTER_FAINT"), monsterName).c_str());
}

void BattleLoopLayer::ShowWhiteOutText()
{
	DialogueBox::SetText(STRINGTABLE->GetString(HASH("BATTLE"), HASH("PLAYER_WHITE_OUT")).c_str());
}

void BattleLoopLayer::ShowMissText(const PocketMonsterEntity* monster) const
{
	// TODO: Nicknames
	const std::string monsterName = STRINGTABLE->GetString(MONSTER_NAME_GROUP, monster->GetNameStringID());
	DialogueBox::SetText(STRINGTABLE->GetDynamicString(HASH("MONSTER_ATTACK_MISS"), monsterName).c_str());
}

void BattleLoopLayer::ShowCriticalHitText() const
{
	DialogueBox::SetText(STRINGTABLE->GetString(HASH("BATTLE"), HASH("MOVE_CRITICAL")).c_str());
}

void BattleLoopLayer::ShowEffectivenessText(const float moveOutcome) const
{
	hash_type stringID = DEFAULT_HASH;
	if (moveOutcome == 0.f)
	{
		stringID = HASH("MOVE_IMMUNE");
	}
	else if (moveOutcome <= 0.5f)
	{
		stringID = HASH("MOVE_NOT_VERY_EFFECTIVE");
	}
	else if (moveOutcome > 1.0f)
	{
		stringID = HASH("MOVE_SUPER_EFFECTIVE");
	}

	ASSERT(stringID != DEFAULT_HASH);

	DialogueBox::SetText(STRINGTABLE->GetString(HASH("BATTLE"), stringID).c_str());
}

void BattleLoopLayer::ShowStatChangeText(const PocketMonsterEntity* monster,
                                         const StatChange::StatStage statChangeInfo,
                                         const bool succeeded)
{
	// TODO: Nicknames
	const std::string monsterName = STRINGTABLE->GetString(MONSTER_NAME_GROUP, monster->GetNameStringID());

	const std::string statString = MonsterStats::GetStatString(statChangeInfo.m_Stat);

	hash_type hashString = DEFAULT_HASH;
	if (statChangeInfo.m_Stages > 0)
	{
		if (succeeded)
		{
			if (statChangeInfo.m_Stages < 2)
			{
				hashString = HASH("MONSTER_STAT_CHANGE_INCREASE_1");
			}
			else if (statChangeInfo.m_Stages < 4)
			{
				hashString = HASH("MONSTER_STAT_CHANGE_INCREASE_2");
			}
			else
			{
				hashString = HASH("MONSTER_STAT_CHANGE_INCREASE_3");
			}
		}
		else
		{
			hashString = HASH("MONSTER_STAT_CHANGE_MAX");
		}
	}
	else
	{
		if (succeeded)
		{
			if (statChangeInfo.m_Stages > -2)
			{
				hashString = HASH("MONSTER_STAT_CHANGE_DECREASE_1");
			}
			else if (statChangeInfo.m_Stages > -4)
			{
				hashString = HASH("MONSTER_STAT_CHANGE_DECREASE_2");
			}
			else
			{
				hashString = HASH("MONSTER_STAT_CHANGE_DECREASE_3");
			}
		}
		else
		{
			hashString = HASH("MONSTER_STAT_CHANGE_MIN");
		}
	}

	DialogueBox::SetText(STRINGTABLE->GetDynamicString(hashString, monsterName.c_str(), statString.c_str()).c_str());
}

void BattleLoopLayer::ShowExperienceText(const monster_xp_t xpGained)
{
	DialogueBox::SetText(STRINGTABLE->GetDynamicString(HASH("MONSTER_GAIN_EXPERIENCE"), std::to_string(xpGained)).c_str());
}

void BattleLoopLayer::ShowLevelUpText(const PocketMonsterEntity* monster, const uint8_t level)
{
	const std::string monsterName = STRINGTABLE->GetString(MONSTER_NAME_GROUP, monster->GetNameStringID());
	DialogueBox::SetText(STRINGTABLE->GetDynamicString(HASH("MONSTER_LEVEL_UP"), monsterName, std::to_string(level)).c_str());
}

Move::Outcome BattleLoopLayer::UseMove(PocketMonsterEntity* attacker,
                                       PocketMonsterEntity* defender,
                                       const uint8_t moveIdx) const
{
	ASSERT(moveIdx < MOVE_COUNT);
	ASSERT(attacker != nullptr);
	ASSERT(defender != nullptr);

	auto* moveComponent = attacker->GetComponent<MoveComponent>();
	ASSERT(moveComponent && moveComponent->CanUseMove(moveIdx));


	const Move::Outcome outcome = moveComponent->UseMove(moveIdx, *defender);

#if BUILD_DEBUG
	std::cout << STRINGTABLE->GetString(MONSTER_NAME_GROUP, attacker->GetNameStringID()) << " stats:\n";
	attacker->GetStats().Log();

	std::cout << "\n";

	std::cout << STRINGTABLE->GetString(MONSTER_NAME_GROUP, defender->GetNameStringID()) << " stats:\n";
	defender->GetStats().Log();
#endif

	return outcome;
}

void BattleLoopLayer::UpdateExperienceBar(monster_xp_t gainedXP, PocketMonsterEntity* playerMonster,
                                          const PocketMonsterEntity* opponentMonster)
{
	const monster_xp_t playerBeforeXP = playerMonster->GetCurrentXP();

	m_battleBeatQueue.emplace(TextBeat{
		.m_BeatName = "AWARDING XP",
		.m_OnShow = [this, gainedXP]()
		{
			ShowExperienceText(gainedXP);
		},
	});

	const uint8_t currentLevel = playerMonster->GetLevel();
	const auto playerExpGroup = playerMonster->GetExperienceGroup();

	const monster_xp_t newValue = playerBeforeXP + gainedXP;
	const uint8_t newLevel = monster_xp::GetLevelForExperience(playerExpGroup, newValue);

	bool firstIncrease = true;
	for (int i = 0; i <= newLevel - currentLevel; ++i)
	{
		if (!firstIncrease)
		{
			m_battleBeatQueue.emplace(TextBeat{
				.m_BeatName = "LEVEL UP TEXT",
				.m_OnShow = [this, playerMonster, currentLevel, i]()
				{
					const uint8_t lvl = currentLevel + static_cast<uint8_t>(i);
					ShowLevelUpText(playerMonster, lvl);
					playerMonster->ApplyLevelUpStats(lvl);
					RefreshPlayerUI(playerMonster, lvl);
				},
			});
		}

		monster_xp_t beforeValue;
		monster_xp_t maxValue;
		if (firstIncrease)
		{
			beforeValue = playerMonster->GetCurrentXP();
			maxValue = monster_xp::GetMaxExperienceForLevel(
				playerExpGroup,
				currentLevel + static_cast<uint8_t>(i) + 1
			);
		}
		else
		{
			beforeValue = monster_xp::GetMaxExperienceForLevel(
				playerExpGroup,
				currentLevel + static_cast<uint8_t>(i)
			);
			maxValue = monster_xp::GetMaxExperienceForLevel(
				playerExpGroup,
				currentLevel + static_cast<uint8_t>(i) + 1
			);
		}

		const monster_xp_t clampedLevelXp = maths::Clamp(newValue, beforeValue, maxValue);

		m_battleBeatQueue.emplace(AnimationBeat{
			.m_BeatName = "XP_BAR",
			.m_Start = [this, firstIncrease, currentLevel, playerExpGroup, beforeValue, clampedLevelXp, maxValue]
			{
				monster_xp_t levelStartXP;
				monster_xp_t normalizedBefore;
				if (firstIncrease)
				{
					levelStartXP = monster_xp::GetMaxExperienceForLevel(
						playerExpGroup, currentLevel);
					normalizedBefore = beforeValue - levelStartXP;
				}
				else
				{
					levelStartXP = beforeValue;
					normalizedBefore = 0; // post-level-up, so just start from empty
				}

				const monster_xp_t levelRange = maxValue - levelStartXP;

				const monster_xp_t normalizedEnd = clampedLevelXp - levelStartXP;

				m_playerExperienceBar.Start(normalizedBefore, normalizedEnd, levelRange, 1.2f);
			},
			.m_IsComplete = [this] { return !m_playerExperienceBar.IsPlaying(); },
			.m_FinishAnimation = [this] { m_playerExperienceBar.Finish(); }
		});

		firstIncrease = false;
	}
}

void BattleLoopLayer::OnMonsterFainted(const BattleState& state,
                                       const PocketMonsterEntity* monster,
                                       const bool isPlayerMonster)
{
	Entity* playerEntity = state.GetGameContext().m_Entities.Get<Entity>(state.GetBattleContext().m_PlayerEntityID);
	Entity* opponentEntity = state.GetGameContext().m_Entities.Get<Entity>(
		state.GetBattleContext().m_OpponentEntityID);

	ASSERT(playerEntity && opponentEntity);

	const MonsterPartyComponent* playerMPC = playerEntity->GetComponent<MonsterPartyComponent>();
	const MonsterPartyComponent* opponentMPC = opponentEntity->GetComponent<MonsterPartyComponent>();

	ASSERT(playerMPC && opponentMPC);

	const bool hasMonstersLeft = isPlayerMonster ? playerMPC->HasMonstersLeft() : opponentMPC->HasMonstersLeft();

	m_battleBeatQueue.emplace(TextBeat{
		.m_BeatName = "MONSTER FAINTED",
		.m_OnShow = [this, monster]()
		{
			ShowFaintText(monster);
		}
	});

	if (!isPlayerMonster)
	{
		auto* playerMonster = playerMPC->GetActiveMonster();

		// TODO:
		const bool isTrainerBattle = false;
		const bool isHoldingLuckyEgg = false;
		const int numberOfPartakingMonsters = 1;

		monster_xp_t gainedXP = monster_xp::GetExperienceGain(
			isTrainerBattle,
			monster->GetExperienceYield(),
			isHoldingLuckyEgg,
			monster->GetLevel(),
			playerMonster->GetLevel(),
			numberOfPartakingMonsters
		);

		UpdateExperienceBar(gainedXP, playerMonster, monster);

		playerMonster->GainExperience(gainedXP);
	}

	if (!hasMonstersLeft)
	{
		// Was it the player?
		if (isPlayerMonster)
		{
			m_battleBeatQueue.emplace(TextBeat{
				.m_BeatName = "FASTEST FAINTED",
				.m_OnShow = [this]()
				{
					ShowWhiteOutText();
				},
				.m_OnDismiss = [this]()
				{
					m_endContext.m_SendPlayerToHospital = true;
					game_events::OnBattleEnd.Fire(m_endContext);
				}
			});
		}
		else
		{
			m_battleBeatQueue.emplace(TextBeat{
				.m_BeatName = "BATTLE END",
				.m_OnShow = [this]()
				{
					game_events::OnBattleEnd.Fire(m_endContext);
				}
			});
		}
	}
}

void BattleLoopLayer::RefreshPlayerUI(PocketMonsterEntity* playerMonster, const uint8_t currentLevel)
{
	auto* battlePanel = UIMANAGER.GetElement<UiPanel>(BATTLE_PANEL_NAME);
	ASSERT(battlePanel);

	UiProgressBar* playerPB = battlePanel->GetChild<UiProgressBar>("PLAYER_HP_BAR");
	ASSERT(playerPB);

	const monster_hp_t playerHP = playerMonster->GetStats().m_HP;
	const monster_hp_t playerMaxHP = playerMonster->GetMaxHP();
	playerPB->SetProgress(playerHP, playerMaxHP, false);

	auto* hpText = battlePanel->GetChild<UiText>("PLAYER_HP_TEXT");
	ASSERT(hpText);
	hpText->SetText("%d/%d", playerHP, playerMaxHP);

	auto* levelText = battlePanel->GetChild<UiText>("PLAYER_LEVEL_TEXT");
	ASSERT(levelText);
	levelText->SetText("%d", currentLevel);
}
