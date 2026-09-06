#include "MonsterAnimation.h"

#include <SFML/Graphics/Sprite.hpp>

#include "../BattleAnimationClips.h"
#include "../../Engine/Asserts.h"
#include "../../Engine/TextureManager.h"
#include "../../Engine/CodeGen/Resources.hpp"

static constexpr std::string_view DAMAGE_SHADER = "HIT_FLASH";
static constexpr std::string_view STAT_CHANGE_SHADER = "STAT_CHANGE";

MonsterAnimation::MonsterAnimation(PocketMonsterEntity* monster):
	m_monster(monster),
	m_animation()
{
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

void MonsterAnimation::UpdateShader(const Keyframe& frame)
{
	return;
}

void MonsterAnimation::ApplyFrame(const Keyframe& frame)
{
	ASSERT(m_monster != nullptr);

	auto* animComp = m_monster->GetComponent<EntityAnimationComponent>();
	ASSERT(animComp != nullptr);

	frame.Print();

	m_monster->SetOffsetPosition(frame.m_Offset);
	m_monster->SetOffsetScale(frame.m_Scale);
	m_monster->SetOffsetRotation(frame.m_Rotation);

	UpdateShader(frame);
}

DamageAnimation::DamageAnimation(PocketMonsterEntity* monster) :
	MonsterAnimation(monster)
{
}

void DamageAnimation::Play()
{
	m_monster->SetCurrentShader(DAMAGE_SHADER);
	MonsterAnimation::Play(battle_animations::HitFlash());
}

void DamageAnimation::UpdateShader(const Keyframe& frame)
{
	m_monster->SetShaderVariable(DAMAGE_SHADER, "flashAmount", frame.m_Opacity);
}

StatAnimation::StatAnimation(PocketMonsterEntity* monster, const AnimationType type) :
	MonsterAnimation(monster),
	m_type(type)
{
}

void StatAnimation::Play()
{
	m_monster->SetCurrentShader(STAT_CHANGE_SHADER);
	MonsterAnimation::Play(m_type == AnimationType::Increase
		                       ? battle_animations::StatIncrease()
		                       : battle_animations::StatDecrease());
}

void StatAnimation::UpdateShader(const Keyframe& frame)
{
	sf::Shader* currentShader = m_monster->GetCurrentShader();

	const sf::Texture* arrowTex = TEXTUREMANAGER.GetTexture(m_type == AnimationType::Increase
		                                                        ? "STAT_INCREASE_ARROWS"
		                                                        : "STAT_DECREASE_ARROWS");
	currentShader->setUniform("arrowTexture", *arrowTex);

	currentShader->setUniform("opacity", frame.m_Opacity);

	const float elapsed = m_animation.GetElapsedTime();

	constexpr float scrollSpeed = 0.4f;
	currentShader->setUniform("scrollOffset",
	                          std::fmod(elapsed * (m_type == AnimationType::Increase ? scrollSpeed : -scrollSpeed), 1.f)
	);

	currentShader->setUniform("arrowTiling", sf::Glsl::Vec2(4.f, 4.f));

	MonsterAnimation::UpdateShader(frame);
}
