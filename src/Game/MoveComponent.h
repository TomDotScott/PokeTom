#ifndef MOVECOMPONENT_H
#define MOVECOMPONENT_H
#include "../Engine/IUpdateable.h"
#include "Move.h"

class Entity;

inline constexpr uint8_t MOVE_COUNT = 4;

class MoveComponent : public IUpdateable
{
public:
	MoveComponent(Entity* owner, const std::array<uint32_t, MOVE_COUNT>& moves);
	void Update(float deltaTime) override;

	bool CanUseMove(uint8_t moveIdx) const;
	bool UseMove(uint8_t moveIdx, Entity& target);

	Move& GetMove(uint8_t moveIdx);
	const Move& GetMove(uint8_t moveIdx) const;

private:
	Entity* m_owner;
	std::array<Move, MOVE_COUNT> m_moves;
};


#endif // MOVECOMPONENT_H
