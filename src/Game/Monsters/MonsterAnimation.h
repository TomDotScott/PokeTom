#ifndef MONSTERHITANIMATION_H
#define MONSTERHITANIMATION_H
#include "PocketMonsterEntity.h"
#include "../../Engine/Animation/KeyFrameAnimation.h"

class MonsterAnimation
{
public:
	MonsterAnimation();
	virtual ~MonsterAnimation() = default;

	virtual void Play() = 0;
	void SetMonster(PocketMonsterEntity* monster);
	void Update(float deltaTime);
	void Finish();
	bool IsPlaying() const;

protected:
	PocketMonsterEntity* m_monster;
	KeyframeAnimation m_animation;

	virtual void UpdateShader(const Keyframe& frame);
	void Play(std::vector<Keyframe> clip);

private:
	void ApplyFrame(const Keyframe& frame);
};

class DamageAnimation : public MonsterAnimation
{
public:
	void Play() override;

protected:
	void UpdateShader(const Keyframe& frame) override;
};

class StatAnimation : public MonsterAnimation
{
public:
	enum class AnimationType
	{
		Increase,
		Decrease
	};

	explicit StatAnimation(AnimationType type);

	void Play() override;

protected:
	void UpdateShader(const Keyframe& frame) override;

private:
	AnimationType m_type;
};

#endif // MONSTERHITANIMATION_H
