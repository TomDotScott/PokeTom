#include "MonsterAnimation.h"

#include <SFML/Graphics/Sprite.hpp>

#include "../BattleAnimationClips.h"
#include "../../Engine/Asserts.h"


MonsterAnimation::MonsterAnimation():
	m_monster(nullptr),
	m_animation()
{
}

void MonsterAnimation::SetMonster(PocketMonsterEntity* monster)
{
	ASSERT(monster != nullptr);
	m_monster = monster;
}

void MonsterAnimation::Play(std::vector<Keyframe> clip)
{
	m_animation.SetFrames(std::move(clip));
	m_animation.Start();
}

void MonsterAnimation::Update(const float deltaTime)
{
	if (!m_animation.IsPlaying())
	{
		return;
	}

	ApplyFrame(m_animation.Update(deltaTime));
}

void MonsterAnimation::Finish()
{
	ApplyFrame(m_animation.Finish());
}

bool MonsterAnimation::IsPlaying() const
{
	return m_animation.IsPlaying();
}

void MonsterAnimation::ApplyFrame(const Keyframe& frame) const
{
	ASSERT(m_monster != nullptr);

	auto* animComp = m_monster->GetComponent<EntityAnimationComponent>();
	ASSERT(animComp != nullptr);

	frame.Print();

	m_monster->SetOffsetPosition(frame.m_Offset);
	m_monster->SetOffsetScale(frame.m_Scale);
	m_monster->SetOffsetRotation(frame.m_Rotation);


	// sf::Sprite& sprite = animComp->GetSprite();
}
