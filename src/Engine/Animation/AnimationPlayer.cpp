#include "AnimationPlayer.h"


AnimationPlayer::AnimationPlayer(const std::shared_ptr<AnimationDictionary>& dict) :
	m_dictionary(dict),
	m_currentAnimName("idle_down"),
	m_frameIndex(0),
	m_frameTime(0.f)
{
}

void AnimationPlayer::PlayAnimation(const std::string& name, const bool forceRestart)
{
	if (!forceRestart && name == m_currentAnimName)
	{
		return;
	}

	m_currentAnimName = name;
	m_frameIndex = 0;
	m_frameTime = 0.f;
}

void AnimationPlayer::Update(const float deltaTime)
{
	if (m_currentAnimName.empty())
	{
		return;
	}

	m_frameTime += deltaTime;

	const Animation& clip = m_dictionary->GetClip(m_currentAnimName);
	const AnimationFrame& frame = clip.m_Frames[m_frameIndex];
	const float duration = static_cast<float>(frame.m_DurationMS) / 1000.f;

	if (m_frameTime < duration)
	{
		return;
	}

	m_frameTime -= duration;
	m_frameIndex++;

	if (m_frameIndex >= clip.m_Frames.size())
	{
		if (clip.m_IsLooping)
		{
			m_frameIndex = 0;
		}
		else
		{
			m_frameIndex = clip.m_Frames.size() - 1;
		}

		if (!clip.m_OnAnimEnd.empty())
		{
			PlayAnimation(clip.m_OnAnimEnd, true);
		}
	}
}

const AnimationFrame& AnimationPlayer::GetCurrentFrame() const
{
	const Animation& clip = m_dictionary->GetClip(m_currentAnimName);
	return clip.m_Frames[m_frameIndex];
}

const sf::Texture& AnimationPlayer::GetTexture() const
{
	return m_dictionary->GetSpriteSheet();
}
