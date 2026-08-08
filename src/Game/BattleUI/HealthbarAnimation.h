#ifndef HEALTHBARANIMATION_H
#define HEALTHBARANIMATION_H

#include "../AnimatedProgressBar.h"
#include "../Monsters/PocketMonsterEntity.h"

class UiText;

class HealthbarAnimation : public AnimatedProgressBar<monster_hp_t>
{
public:
	enum class eAnimationType
	{
		Player,
		Opponent
	};

	explicit HealthbarAnimation(eAnimationType type);
	virtual ~HealthbarAnimation() = default;

	void SetMonster(PocketMonsterEntity* monster);

	void Start(monster_hp_t monsterHealthBefore);

private:
	UiText* m_hpText;
	PocketMonsterEntity* m_monster;
	eAnimationType m_type;

	void UpdateFill() const override;
};
#endif // HEALTHBARANIMATION_H
