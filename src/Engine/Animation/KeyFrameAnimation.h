#ifndef KEYFRAMEANIMATION_H
#define KEYFRAMEANIMATION_H

#include <vector>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>


// All of these fields are RELATIVE to the sprite's base sf::Transform
struct Keyframe
{
	float m_Time = 0.f;
	sf::Vector2f m_Offset = { 0.f, 0.f };
	sf::Vector2f m_Scale = { 0.f, 0.f };
	sf::Angle m_Rotation = sf::Angle::Zero;
	float m_Opacity = 1.f;
	sf::Color m_Colour = sf::Color::White;

	void Print() const
	{
		printf("KEY_FRAME_DATA: Time=%f, offset={%f,%f}, scale={%f,%f}, rotation={%f}, opacity={%f}\n", m_Time, m_Offset.x, m_Offset.y, m_Scale.x, m_Scale.y, m_Rotation.asDegrees(), m_Opacity);
	}
};

class KeyframeAnimation
{
public:
	KeyframeAnimation();

	void SetFrames(std::vector<Keyframe> keyframes);

	void Start();

	// Returns the evaluated frame for the current elapsed time so the
	// caller can apply it to their base target
	Keyframe Update(float deltaTime);
	Keyframe Finish();

	bool IsPlaying() const;

private:
	std::vector<Keyframe> m_keyframes;
	float m_elapsedTime;
	bool m_playing;

	Keyframe Evaluate(float t) const;
};

#endif
