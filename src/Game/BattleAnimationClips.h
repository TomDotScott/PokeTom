#ifndef BATTLEANIMATIONCLIPS_H
#define BATTLEANIMATIONCLIPS_H
#include "../Engine/Animation/KeyFrameAnimation.h"

namespace battle_animations
{
	inline std::vector<Keyframe> HitFlash()
	{
		return {
			Keyframe{ .m_Time = 0.00f, .m_Opacity = 0.f },
			Keyframe{ .m_Time = 0.08f, .m_Opacity = 1.f },
			Keyframe{ .m_Time = 0.16f, .m_Opacity = 0.f },
			Keyframe{ .m_Time = 0.24f, .m_Opacity = 1.f },
			Keyframe{ .m_Time = 0.32f, .m_Opacity = 0.f },
		};
	}

	inline std::vector<Keyframe> StatIncrease()
	{
		return {
			Keyframe{ .m_Time = 0.0f, .m_Scale = { 1.0f, 1.0f }, .m_Opacity = 0.f },
			Keyframe{ .m_Time = 0.1f, .m_Scale = { 1.1f, 1.1f }, .m_Opacity = 1.f },
			Keyframe{ .m_Time = 0.9f, .m_Scale = { 1.1f, 1.1f }, .m_Opacity = 1.f },
			Keyframe{ .m_Time = 1.0f, .m_Scale = { 1.0f, 1.0f }, .m_Opacity = 0.f },
		};
	}

	inline std::vector<Keyframe> StatDecrease()
	{
		return {
			Keyframe{ .m_Time = 0.0f, .m_Scale = { 1.0f, 1.0f }, .m_Opacity = 0.f },
			Keyframe{ .m_Time = 0.1f, .m_Scale = { 0.9f, 0.9f }, .m_Opacity = 1.f },
			Keyframe{ .m_Time = 0.9f, .m_Scale = { 0.9f, 0.9f }, .m_Opacity = 1.f },
			Keyframe{ .m_Time = 1.0f, .m_Scale = { 1.0f, 1.0f }, .m_Opacity = 0.f },
		};
	}

	inline std::vector<Keyframe> Burn()
	{
		return {
			Keyframe{.m_Time = 0.00f, .m_Offset = { 0.f, 0.f },  .m_Opacity = 0.f },
			Keyframe{.m_Time = 0.10f, .m_Offset = { -2.f, 0.f }, .m_Opacity = 0.5f },
			Keyframe{.m_Time = 0.20f, .m_Offset = { 2.f, 0.f },  .m_Opacity = 1.f },
			Keyframe{.m_Time = 0.30f, .m_Offset = { -1.f, 0.f }, .m_Opacity = 0.5f },
			Keyframe{.m_Time = 0.40f, .m_Offset = { 0.f, 0.f },  .m_Opacity = 0.f },
		};
	}

	inline std::vector<Keyframe> Poison()
	{
		return {
			Keyframe{.m_Time = 0.0f, .m_Offset = { 0.f, 0.f }, .m_Scale = { 1.0f, 1.0f }, .m_Opacity = 0.f },
			Keyframe{.m_Time = 0.3f, .m_Offset = { 0.f, 6.f }, .m_Scale = { 0.8f, 1.0f }, .m_Opacity = 1.f },
			Keyframe{.m_Time = 0.6f, .m_Offset = { 0.f, 0.f }, .m_Scale = { 1.0f, 1.0f }, .m_Opacity = 0.f },
		};
	}

	inline std::vector<Keyframe> Paralysis()
	{
		return {
			Keyframe{.m_Time = 0.00f, .m_Rotation = sf::Angle::Zero,   .m_Opacity = 0.f },
			Keyframe{.m_Time = 0.05f, .m_Rotation = sf::degrees(-8.f), .m_Opacity = 1.f },
			Keyframe{.m_Time = 0.10f, .m_Rotation = sf::degrees(8.f),  .m_Opacity = 1.f },
			Keyframe{.m_Time = 0.15f, .m_Rotation = sf::degrees(-4.f), .m_Opacity = 0.5f },
			Keyframe{.m_Time = 0.20f, .m_Rotation = sf::Angle::Zero,   .m_Opacity = 0.f },
		};
	}

	inline std::vector<Keyframe> Freeze()
	{
		return {
			Keyframe{.m_Time = 0.00f, .m_Offset = { 0.f, 0.f },  .m_Opacity = 0.f },
			Keyframe{.m_Time = 0.10f, .m_Offset = { -1.f, 0.f }, .m_Opacity = 0.5f },
			Keyframe{.m_Time = 0.20f, .m_Offset = { 1.f, 0.f },  .m_Opacity = 1.f },
			Keyframe{.m_Time = 0.30f, .m_Offset = { -1.f, 0.f }, .m_Opacity = 0.5f },
			Keyframe{.m_Time = 0.40f, .m_Offset = { 0.f, 0.f },  .m_Opacity = 0.f },
		};
	}

	inline std::vector<Keyframe> Sleep()
	{
		return {
			Keyframe{.m_Time = 0.0f, .m_Offset = { 0.f, 0.f }, .m_Opacity = 0.f },
			Keyframe{.m_Time = 0.5f, .m_Offset = { 0.f, 3.f }, .m_Opacity = 0.f },
			Keyframe{.m_Time = 1.0f, .m_Offset = { 0.f, 0.f }, .m_Opacity = 0.f },
		};
	}
}

#endif // BATTLEANIMATIONCLIPS_H
