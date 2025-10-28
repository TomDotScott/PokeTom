#ifndef PLAYER_H
#define PLAYER_H
#include "../Engine/Gameobject.h"
#include "../Engine/Updateable.h"
#include "../Engine/Input/InputMapper.h"

class Player final : public GameObject, Updateable
{
public:
	Player();

	void Update(float deltaTime) override;

private:
	enum eInputs : uint8_t
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
	};

	InputMapper m_mapper;
};

#endif
