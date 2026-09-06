#include "KeyFrameAnimation.h"

#include <algorithm>

#include "../Asserts.h"
#include "../Maths.h"

KeyframeAnimation::KeyframeAnimation() :
	m_keyframes(),
	m_elapsedTime(0),
	m_playing(false)
{
}

void KeyframeAnimation::SetFrames(std::vector<Keyframe> keyframes)
{
	ASSERT(!keyframes.empty());
	std::ranges::sort(keyframes, [](const auto& a, const auto& b) { return a.m_Time < b.m_Time; });
	m_keyframes = std::move(keyframes);
}

void KeyframeAnimation::Start()
{
	ASSERT(!m_keyframes.empty());

	m_elapsedTime = 0.f;
	m_playing = true;
}

Keyframe KeyframeAnimation::Update(const float deltaTime)
{
	if (!m_playing)
	{
		return m_keyframes.back();
	}

	m_elapsedTime += deltaTime;

	float duration = m_keyframes.back().m_Time;
	if (m_elapsedTime >= duration)
	{
		m_playing = false;
		m_elapsedTime = duration;
	}

	return Evaluate(m_elapsedTime);
}

Keyframe KeyframeAnimation::Finish()
{
	m_playing = false;
	if (m_keyframes.empty())
	{
		m_elapsedTime = 0.f;
		return Keyframe{};
	}

	m_elapsedTime = m_keyframes.back().m_Time;
	return m_keyframes.back();
}

bool KeyframeAnimation::IsPlaying() const
{
	return m_playing;
}

float KeyframeAnimation::GetElapsedTime() const
{
	return m_elapsedTime;
}

Keyframe KeyframeAnimation::Evaluate(float t) const
{
	ASSERT(!m_keyframes.empty());
	t = maths::Clamp(t, m_keyframes.front().m_Time, m_keyframes.back().m_Time);

	// Work out our current and previous keyframe so we can lerp between them
	for (int i = 1; i < m_keyframes.size(); ++i)
	{
		const Keyframe& prev = m_keyframes[i - 1];
		const Keyframe& next = m_keyframes[i];

		if (t <= next.m_Time)
		{
			const float segmentLength = next.m_Time - prev.m_Time;
			const float scaledLength = (segmentLength > 0.f) ? (t - prev.m_Time) / segmentLength : 1.f;

			return {
				.m_Time = t,
				.m_Offset = maths::Lerp(prev.m_Offset, next.m_Offset, scaledLength),
				.m_Scale = maths::Lerp(prev.m_Scale, next.m_Scale, scaledLength),
				.m_Rotation = maths::Lerp(prev.m_Rotation, next.m_Rotation, scaledLength),
				.m_Opacity = maths::Lerp(prev.m_Opacity, next.m_Opacity, scaledLength),
				.m_Colour = maths::Lerp(prev.m_Colour, next.m_Colour, scaledLength),
			};
		}
	}

	return m_keyframes.back();
}
