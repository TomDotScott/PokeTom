#ifndef MOVECOMPONENT_H
#define MOVECOMPONENT_H
#include "../Engine/IUpdateable.h"
#include "Move.h"

class Entity;

class MoveComponent : public IUpdateable
{
public:
	MoveComponent(Entity* owner, const std::array<uint32_t, 4>& moves);
	void Update(float deltaTime) override;

	bool CanUseMove(uint8_t moveIdx) const;
	bool UseMove(uint8_t moveIdx, Entity& target);

private:
	Entity* m_owner;
	std::array<Move, 4> m_moves;
};


#endif // MOVECOMPONENT_H
