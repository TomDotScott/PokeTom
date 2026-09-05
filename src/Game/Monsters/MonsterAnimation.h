#ifndef MONSTERHITANIMATION_H
#define MONSTERHITANIMATION_H
#include "PocketMonsterEntity.h"
#include "../../Engine/Animation/KeyFrameAnimation.h"

class MonsterAnimation
{
public:
	MonsterAnimation();
	void SetMonster(PocketMonsterEntity* monster);
	void Play(std::vector<Keyframe> clip);
	void Update(float deltaTime);
	void Finish();
	bool IsPlaying() const;

private:
	PocketMonsterEntity* m_monster;
	KeyframeAnimation m_animation;

	void ApplyFrame(const Keyframe& frame) const;
};

#endif // MONSTERHITANIMATION_H
