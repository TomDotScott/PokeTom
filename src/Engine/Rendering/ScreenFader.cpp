#include "ScreenFader.h"
#include <algorithm>
#include "../Maths.h"

ScreenFader::ScreenFader() :
	m_alpha(0),
	m_progress(0.f),
	m_duration(0.f),
	m_holdTime(0.f),
	m_fadeType(FadeType::FadeIn)
{
}

void ScreenFader::Update(const float deltaTime)
{
	const float fullDuration = m_duration + m_holdTime;

	m_progress += deltaTime;

	m_alpha = maths::Lerp(0, 255, std::clamp(m_progress / fullDuration, 0.0f, 1.f));

	if (m_fadeType == FadeType::FadeIn)
	{
		m_alpha = 255 - m_alpha;
	}
}

void ScreenFader::StartFade(const FadeType type, const float duration, const float holdTime)
{
	if (type == FadeType::FadeIn)
	{
		m_alpha = 255;
	}
	else if (type == FadeType::FadeOut)
	{
		m_alpha = 0;
	}

	m_fadeType = type;
	m_progress = 0.f;
	m_duration = duration;
	m_holdTime = holdTime;
}

bool ScreenFader::FadeInProgress() const
{
	return m_progress < m_duration + m_holdTime;
}

ScreenFader::FadeType ScreenFader::GetCurrentFadeType() const
{
	return m_fadeType;
}

uint8_t ScreenFader::GetScreenAlpha() const
{
	return m_alpha;
}
