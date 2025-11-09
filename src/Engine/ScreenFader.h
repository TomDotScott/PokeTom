#ifndef SCREENFADER_H
#define SCREENFADER_H
#include <SFML/Graphics/RenderWindow.hpp>

#include "Timer.h"

class ScreenFader
{
public:
	enum class FadeType
	{
		FadeIn,
		FadeOut
	};

	ScreenFader();

	void Update(float deltaTime);

	void StartFade(FadeType type, float duration, float holdTime);
	bool FadeInProgress() const;

	FadeType GetCurrentFadeType() const;

	uint8_t GetScreenAlpha() const;

private:
	uint8_t m_alpha;
	float m_progress;
	float m_duration;
	float m_holdTime;
	FadeType m_fadeType;
};

#endif // SCREENFADER_H
