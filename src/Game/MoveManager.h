#ifndef MOVEMANAGER_H
#define MOVEMANAGER_H

#include <unordered_map>

#include "Move.h"
#include "../Engine/Factory.h"

class MoveManager : protected Factory<MoveManager>
{
public:
	static MoveManager* Get();

	const Move& GetMove(uint32_t moveID) const;

protected:
	friend class Factory<MoveManager>;
	bool Init() override;

private:
	std::unordered_map<uint32_t, Move> m_moves;

	bool LoadJSON();
	MoveManager() = default;
};

#endif // MOVEMANAGER_H
