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
}

#endif // BATTLEANIMATIONCLIPS_H
