#ifndef TIMER_H
#define TIMER_H
#include <chrono>

class Timer
{

public:
	static Timer& Get();

	float DeltaTime() const;

	float Fps() const;

	void Update();

private:
	Timer();

	std::chrono::high_resolution_clock::time_point m_timeLastUpdate;
	std::chrono::high_resolution_clock::duration m_delta;
};
#endif
