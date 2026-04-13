#ifndef ANIMATIONPLAYER_H
#define ANIMATIONPLAYER_H
#include <memory>

#include "AnimDef.h"

class AnimationPlayer
{
public:
	AnimationPlayer(const std::shared_ptr<AnimationDictionary>& dict);

	void PlayAnimation(const std::string_view& name, bool forceRestart);
	void Update(float deltaTime);
	const AnimationFrame& GetCurrentFrame() const;
	const std::string& GetDictionarySpritesheetResourceName() const;

private:
	std::shared_ptr<AnimationDictionary> m_dictionary;
	std::string m_currentAnimName;
	size_t m_frameIndex;
	float m_frameTime;
};

#endif // ANIMATIONPLAYER_H
