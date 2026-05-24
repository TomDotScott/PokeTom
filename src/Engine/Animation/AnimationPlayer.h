#ifndef ANIMATIONPLAYER_H
#define ANIMATIONPLAYER_H
#include <memory>

#include "AnimDef.h"

class AnimationPlayer
{
public:
	AnimationPlayer(const std::shared_ptr<AnimationDictionary>& dict);

	bool CanPlayAnimation(const hash_type& name) const;
	void PlayAnimation(const hash_type& name, bool forceRestart);

	void Update(float deltaTime);

	const AnimationFrame& GetCurrentFrame() const;
	const Animation& GetCurrentClip() const;
	const hash_type& GetDictionarySpritesheetResourceName() const;

private:
	std::shared_ptr<AnimationDictionary> m_dictionary;
	hash_type m_currentAnimName;
	size_t m_frameIndex;
	float m_frameTime;
};

#endif // ANIMATIONPLAYER_H
