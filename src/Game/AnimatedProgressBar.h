#ifndef ANIMATEDPROGRESSBAR_H
#define ANIMATEDPROGRESSBAR_H

#include "../Engine/Asserts.h"
#include "../Engine/Maths.h"
#include "../Engine/UI/UiProgressBar.h"

template <typename T>
class AnimatedProgressBar
{
public:
	explicit AnimatedProgressBar(UiProgressBar* barToAnimate) :
		m_maximumValue(T()),
		m_valueBefore(T()),
		m_currentValue(T()),
		m_progressBar(barToAnimate),
		m_duration(0.f),
		m_currentTime(0.f),
		m_finished(true)
	{
	}

	virtual ~AnimatedProgressBar() = default;

	void Start(T valueBefore, T newValue, T maxValue, const float duration)
	{
		m_finished = false;
		m_currentTime = 0.f;

		m_valueBefore = valueBefore;
		m_currentValue = newValue;
		m_maximumValue = maxValue;

		m_duration = duration;

		UpdateFill();
	}


	void Update(const float deltaTime)
	{
		if (m_finished)
		{
			return;
		}

		if (m_currentTime > m_duration)
		{
			Finish();
			return;
		}

		m_currentTime += deltaTime;
		UpdateFill();
	}

	void Finish()
	{
		m_currentTime = m_duration;

		UpdateFill();

		m_finished = true;
	}

	bool IsPlaying() const
	{
		return !m_finished;
	}

protected:
	float CalculateFill() const
	{
		return maths::Lerp(static_cast<float>(m_valueBefore) / static_cast<float>(m_maximumValue),
		                   static_cast<float>(m_currentValue) / static_cast<float>(m_maximumValue),
		                   m_currentTime / m_duration
		);
	}

	virtual void UpdateFill() const
	{
		ASSERT(m_progressBar);
		m_progressBar->SetProgress(CalculateFill());
	}

	T m_maximumValue;

private:
	T m_valueBefore;
	T m_currentValue;

	UiProgressBar* m_progressBar;

	float m_duration;
	float m_currentTime;

	bool m_finished;
};
#endif // ANIMATEDPROGRESSBAR_H
