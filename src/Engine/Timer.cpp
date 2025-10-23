#include "Timer.h"

Timer& Timer::Get()
{
	static Timer* instance = nullptr;

	if (instance == nullptr)
	{
		instance = new Timer();
	}

	return *instance;
}

float Timer::DeltaTime() const
{
	return m_delta.count() / powf(10.f, 9.f);
}

float Timer::Fps() const
{
	return 1.0f / DeltaTime();
}

Timer::Timer() :
	m_timeLastUpdate(std::chrono::high_resolution_clock::now()),
	m_delta()
{
}

void Timer::Update()
{
	const std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	m_delta = currentTime - m_timeLastUpdate;

	m_timeLastUpdate = currentTime;
}